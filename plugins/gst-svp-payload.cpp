/*
 * Copyright (C) 2019 RDK Management
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "gst-svp-payload.h"

#include <gst_svp_meta.h>

#include <dlfcn.h>

struct _SvpPayloadPrivate {
  void* svpCtx;
  gboolean decryptToHostSet;
  gboolean decryptToHost;
};

GST_DEBUG_CATEGORY_STATIC(svp_payload_debug_category);
#define GST_CAT_DEFAULT svp_payload_debug_category

#define svp_payload_parent_class parent_class
G_DEFINE_TYPE_WITH_PRIVATE(SvpPayload, svp_payload, GST_TYPE_BASE_TRANSFORM);

static GstStaticPadTemplate sinkTemplate =
  GST_STATIC_PAD_TEMPLATE(
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(payloadCapsSink));

static GstStaticPadTemplate srcTemplate =
  GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(payloadCapsSrc));

static GstFlowReturn svpPayloadTransform(GstBaseTransform* base, GstBuffer* in, GstBuffer* out);
static GstFlowReturn svpPayloadTransformInPlace(GstBaseTransform* base, GstBuffer* buffer);
static GstFlowReturn svpPayloadPrepareOutputBuffer (GstBaseTransform * trans, GstBuffer * inbuf, GstBuffer ** outbuf);
static GstCaps* svpPayloadTransformCaps(GstBaseTransform*, GstPadDirection, GstCaps*, GstCaps*);
static GstStateChangeReturn svpPayloadChangeState(GstElement* element, GstStateChange transition);
static gboolean has_decrypt_to_host(GstCaps *caps);
static void svp_payload_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec);
static void svpPayloadUpdateDecryptToHost(SvpPayloadPrivate* priv, gboolean decryptToHost);

enum {
    PROP_0,
    PROP_BUFFERS_AVAILABLE
};

static void svp_payload_class_init(SvpPayloadClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->get_property = svp_payload_get_property;

    g_object_class_install_property(gobject_class, PROP_BUFFERS_AVAILABLE,
        g_param_spec_boolean("buffers-available", "Buffers Available",
            "Whether SVP video buffers are available for use",
            FALSE, (GParamFlags) (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

    GstElementClass* elementClass = GST_ELEMENT_CLASS(klass);
    elementClass->change_state = GST_DEBUG_FUNCPTR(svpPayloadChangeState);
    gst_element_class_add_pad_template(elementClass, gst_static_pad_template_get(&sinkTemplate));
    gst_element_class_add_pad_template(elementClass, gst_static_pad_template_get(&srcTemplate));

    gst_element_class_set_static_metadata(
        elementClass,
        "SVP payloader.",
        GST_ELEMENT_FACTORY_KLASS_PAYLOADER,
        "Transfers media into SVP memory.",
        "Comcast");

    GST_DEBUG_CATEGORY_INIT(svp_payload_debug_category,
        "svppay", 0, "SVP payloader");

    GstBaseTransformClass* baseTransformClass = GST_BASE_TRANSFORM_CLASS(klass);
    baseTransformClass->transform = GST_DEBUG_FUNCPTR(svpPayloadTransform);
    baseTransformClass->transform_ip = GST_DEBUG_FUNCPTR(svpPayloadTransformInPlace);
    baseTransformClass->prepare_output_buffer = GST_DEBUG_FUNCPTR(svpPayloadPrepareOutputBuffer);
    baseTransformClass->transform_caps = GST_DEBUG_FUNCPTR(svpPayloadTransformCaps);
    baseTransformClass->transform_ip_on_passthrough = FALSE;
}

static void svp_payload_init(SvpPayload* self)
{
    SvpPayloadPrivate* priv = (SvpPayloadPrivate*)svp_payload_get_instance_private(self);
    priv->svpCtx = NULL;
    priv->decryptToHostSet = FALSE;
    priv->decryptToHost = FALSE;
    self->priv = priv;
    self->decrypt_to_host_enabled = FALSE;

    if (!gst_svp_ext_get_context(&priv->svpCtx, Server, 0))
    {
      GST_ERROR("Failed to get context for payloader\n");
    }

    GstBaseTransform* base = GST_BASE_TRANSFORM(self);
    gst_base_transform_set_in_place(base, FALSE);
    gst_base_transform_set_passthrough(base, FALSE);
    gst_base_transform_set_gap_aware(base, FALSE);
}

static void svp_payload_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    SvpPayload* self = SVP_PAYLOAD(object);
    SvpPayloadPrivate* priv = self->priv;

    switch (prop_id) {
    case PROP_BUFFERS_AVAILABLE:
        if (priv->svpCtx) {
            g_value_set_boolean(value, svp_pipeline_buffers_available(priv->svpCtx, Video));
        } else {
            g_value_set_boolean(value, FALSE);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static GstFlowReturn svpPayloadTransform(GstBaseTransform* base, GstBuffer* inbuf, GstBuffer* outbuf)
{
    GST_DEBUG_OBJECT(base, "Transform inbuf=(%" GST_PTR_FORMAT "), outbuf=(%" GST_PTR_FORMAT ")", inbuf, outbuf);

    SvpPayload* self = SVP_PAYLOAD(base);
    SvpPayloadPrivate* priv = (SvpPayloadPrivate*)svp_payload_get_instance_private(self);

    const gboolean decryptToHost = self->decrypt_to_host_enabled ? TRUE : FALSE;
    svpPayloadUpdateDecryptToHost(priv, decryptToHost);

    if ( gst_svp_is_buffer_using_secure_memory(priv->svpCtx, inbuf) || self->decrypt_to_host_enabled ) {
        if (self->decrypt_to_host_enabled) {
            GST_DEBUG_OBJECT(base, "Decrypt to host enabled, passthrough");
        } else {
            GST_DEBUG_OBJECT(base, "Already in secmem, passthrough");
        }
        g_return_val_if_fail((inbuf == outbuf), GST_FLOW_ERROR);
       return GST_FLOW_OK;
    }

    if (!gst_buffer_svp_transform_from_cleardata(priv->svpCtx, outbuf, Video))
      return GST_FLOW_ERROR;

    GST_DEBUG_OBJECT(base, "Successfully transformed cleardata");
    return GST_FLOW_OK;
}

static GstFlowReturn svpPayloadTransformInPlace(GstBaseTransform* base, GstBuffer* buffer)
{
    GST_DEBUG_OBJECT(base, "Transform in place buf=(%" GST_PTR_FORMAT ")", buffer);

    SvpPayload* self = SVP_PAYLOAD(base);
    SvpPayloadPrivate* priv = (SvpPayloadPrivate*)svp_payload_get_instance_private(self);

    const gboolean decryptToHost = self->decrypt_to_host_enabled ? TRUE : FALSE;
    svpPayloadUpdateDecryptToHost(priv, decryptToHost);

    if ( gst_svp_is_buffer_using_secure_memory(priv->svpCtx, buffer) || self->decrypt_to_host_enabled ) {
        if (self->decrypt_to_host_enabled) {
            GST_DEBUG_OBJECT(base, "Decrypt to host enabled, passthrough");
        } else {
            GST_DEBUG_OBJECT(base, "Already in secmem, passthrough");
        }
       return GST_FLOW_OK;
    }

    if (!gst_buffer_svp_transform_from_cleardata(priv->svpCtx, buffer, Video))
        return GST_FLOW_ERROR;

    GST_DEBUG_OBJECT(base, "Successfully transformed cleardata in place");
    return GST_FLOW_OK;
}

static void svpPayloadUpdateDecryptToHost(SvpPayloadPrivate* priv, gboolean decryptToHost)
{
    if (priv->decryptToHostSet && priv->decryptToHost == decryptToHost)
        return;

    if (gst_svp_ext_context_process_store_set_value(priv->svpCtx,
                                                     "decryptToHost",
                                                     &decryptToHost,
                                                     sizeof(decryptToHost))) {
        priv->decryptToHost = decryptToHost;
        priv->decryptToHostSet = TRUE;
    }
}

static GstFlowReturn svpPayloadPrepareOutputBuffer (GstBaseTransform * base, GstBuffer * inbuf, GstBuffer ** outbuf)
{
    SvpPayload* self = SVP_PAYLOAD(base);
    SvpPayloadPrivate* priv = (SvpPayloadPrivate*)svp_payload_get_instance_private(self);

    if ( gst_svp_is_buffer_using_secure_memory(priv->svpCtx, inbuf) || self->decrypt_to_host_enabled ) {
       *outbuf = inbuf;
       return GST_FLOW_OK;
    }

    if ( gst_base_transform_is_in_place(base) == TRUE ) {
       GST_DEBUG_OBJECT(base, "Prepare buffer, call parent func");
       return GST_BASE_TRANSFORM_CLASS(parent_class)->prepare_output_buffer(base, inbuf, outbuf);
    } else {
       GST_DEBUG_OBJECT(base, "Prepare buffer, copy");
       *outbuf = gst_buffer_copy (inbuf);
       return GST_FLOW_OK;
    }
}

static GstCaps* svpPayloadTransformCaps(GstBaseTransform* base, GstPadDirection direction, GstCaps* caps, GstCaps* filter)
{
    if (direction == GST_PAD_UNKNOWN)
        return NULL;

    SvpPayload* self = SVP_PAYLOAD(base);
    SvpPayloadPrivate* priv = (SvpPayloadPrivate*)svp_payload_get_instance_private(self);

    GST_DEBUG_OBJECT(self, "Transform in direction: %s, caps %" GST_PTR_FORMAT ", filter %" GST_PTR_FORMAT,
                     direction == GST_PAD_SINK ? "GST_PAD_SINK" : "GST_PAD_SRC", caps, filter);

    GstCaps *ret = NULL;

    self->decrypt_to_host_enabled = has_decrypt_to_host(caps);

    if (direction == GST_PAD_SINK) {
      ret = gst_caps_ref (caps);
      // avoid transforming caps if decrypt to host is enabled
      if (self->decrypt_to_host_enabled == FALSE) {
        if(!gst_svp_ext_transform_caps(&ret, FALSE)) {
          gst_caps_unref(caps);
          GST_ERROR_OBJECT(self, "Caps transform failed");
          return NULL;
        }
      }
    } else {
      ret = gst_caps_copy(caps);
      gst_svp_ext_transform_caps_clear(priv->svpCtx, ret);
    }

    if (filter) {
      GstCaps* intersection;
      intersection = gst_caps_intersect_full (ret, filter, GST_CAPS_INTERSECT_FIRST);
      gst_caps_unref(ret);
      ret = intersection;
    }

    GST_DEBUG_OBJECT(self, "return caps %" GST_PTR_FORMAT, ret);

    return ret;
}

static gboolean has_decrypt_to_host(GstCaps *caps) {
    if (!caps) return FALSE;
    guint n = gst_caps_get_size(caps);
    for (guint i = 0; i < n; ++i) {
        const GstStructure *s = gst_caps_get_structure(caps, i);
        gboolean decrypt_to_host = FALSE;
        if (gst_structure_get_boolean(s, "decrypt-to-host", &decrypt_to_host) && decrypt_to_host) {
            return TRUE;
        }
    }
    return FALSE;
}

static GstStateChangeReturn svpPayloadChangeState(GstElement* element, GstStateChange transition)
{
    SvpPayload* self = SVP_PAYLOAD(element);
    SvpPayloadPrivate* priv = (SvpPayloadPrivate*)svp_payload_get_instance_private(self);

    switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
        GST_DEBUG_OBJECT(self, "NULL->READY");
        if (priv->svpCtx == NULL)
          gst_svp_ext_get_context(&priv->svpCtx, Server, 0);
        break;
    default:
        break;
    }

    GstStateChangeReturn result = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);

    switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
        GST_DEBUG_OBJECT(self, "READY->NULL");
        gst_svp_ext_free_context(priv->svpCtx);
        priv->svpCtx = NULL;
        break;
    default:
        break;
    }

    return result;
}

static gboolean svp_payload_plugin_init (GstPlugin * plugin)
{
   return gst_element_register (plugin,
                                "svppay",
                                GST_RANK_MARGINAL,
                                svp_payload_get_type());
}

#define PACKAGE_ORIGIN "https://github.com/rdkcentral/gst-svp-ext-community"
#define PACKAGE "gst_svp_ext"

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    svppay,
    "Transfers media into SVP memory",
    svp_payload_plugin_init,
    "0.1",
    "LGPL",
    PACKAGE,
    PACKAGE_ORIGIN )
