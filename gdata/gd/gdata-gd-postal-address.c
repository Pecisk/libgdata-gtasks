/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * GData Client
 * Copyright (C) Philip Withnall 2009 <philip@tecnocode.co.uk>
 *
 * GData Client is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GData Client is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GData Client.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gdata-gd-postal-address
 * @short_description: GData postal address element
 * @stability: Unstable
 * @include: gdata/gd/gdata-gd-postal-address.h
 *
 * #GDataGDPostalAddress represents an "postal" element from the
 * <ulink type="http" url="http://code.google.com/apis/gdata/docs/1.0/elements.html#gdPostalAddress">GData specification</ulink>.
 **/

#include <glib.h>
#include <libxml/parser.h>

#include "gdata-gd-postal-address.h"
#include "gdata-parsable.h"
#include "gdata-parser.h"

static void gdata_gd_postal_address_finalize (GObject *object);
static void gdata_gd_postal_address_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void gdata_gd_postal_address_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static gboolean pre_parse_xml (GDataParsable *parsable, xmlDoc *doc, xmlNode *root_node, gpointer user_data, GError **error);
static gboolean parse_xml (GDataParsable *parsable, xmlDoc *doc, xmlNode *node, gpointer user_data, GError **error);
static void pre_get_xml (GDataParsable *parsable, GString *xml_string);
static void get_xml (GDataParsable *parsable, GString *xml_string);
static void get_namespaces (GDataParsable *parsable, GHashTable *namespaces);

struct _GDataGDPostalAddressPrivate {
	gchar *address;
	gchar *relation_type;
	gchar *label;
	gboolean is_primary;
};

enum {
	PROP_ADDRESS = 1,
	PROP_RELATION_TYPE,
	PROP_LABEL,
	PROP_IS_PRIMARY
};

G_DEFINE_TYPE (GDataGDPostalAddress, gdata_gd_postal_address, GDATA_TYPE_PARSABLE)
#define GDATA_GD_POSTAL_ADDRESS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDATA_TYPE_GD_POSTAL_ADDRESS, GDataGDPostalAddressPrivate))

static void
gdata_gd_postal_address_class_init (GDataGDPostalAddressClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GDataParsableClass *parsable_class = GDATA_PARSABLE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GDataGDPostalAddressPrivate));

	gobject_class->get_property = gdata_gd_postal_address_get_property;
	gobject_class->set_property = gdata_gd_postal_address_set_property;
	gobject_class->finalize = gdata_gd_postal_address_finalize;

	parsable_class->pre_parse_xml = pre_parse_xml;
	parsable_class->parse_xml = parse_xml;
	parsable_class->pre_get_xml = pre_get_xml;
	parsable_class->get_xml = get_xml;
	parsable_class->get_namespaces = get_namespaces;
	parsable_class->element_name = "postalAddress";
	parsable_class->element_namespace = "gd";

	/**
	 * GDataGDPostalAddress:address:
	 *
	 * The postal address itself.
	 *
	 * For more information, see the
	 * <ulink type="http" url="http://code.google.com/apis/gdata/docs/1.0/elements.html#gdPostalAddress">GData specification</ulink>.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_ADDRESS,
				g_param_spec_string ("address",
					"Address", "The postal address itself.",
					NULL,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * GDataGDPostalAddress:relation-type:
	 *
	 * A programmatic value that identifies the type of postal address.
	 *
	 * For more information, see the
	 * <ulink type="http" url="http://code.google.com/apis/gdata/docs/1.0/elements.html#gdPostalAddress">GData specification</ulink>.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_RELATION_TYPE,
				g_param_spec_string ("relation-type",
					"Relation type", "A programmatic value that identifies the type of postal address.",
					NULL,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * GDataGDPostalAddress:label:
	 *
	 * A simple string value used to name this postal address. It allows UIs to display a label such as "Work", "Personal", "Preferred", etc.
	 *
	 * For more information, see the
	 * <ulink type="http" url="http://code.google.com/apis/gdata/docs/1.0/elements.html#gdPostalAddress">GData specification</ulink>.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_LABEL,
				g_param_spec_string ("label",
					"Label", "A simple string value used to name this postal address.",
					NULL,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * GDataGDPostalAddress:is-primary:
	 *
	 * Indicates which postal address out of a group is primary.
	 *
	 * For more information, see the
	 * <ulink type="http" url="http://code.google.com/apis/gdata/docs/1.0/elements.html#gdPostalAddress">GData specification</ulink>.
	 *
	 * Since: 0.4.0
	 **/
	g_object_class_install_property (gobject_class, PROP_IS_PRIMARY,
				g_param_spec_boolean ("is-primary",
					"Primary?", "Indicates which postal address out of a group is primary.",
					FALSE,
					G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
gdata_gd_postal_address_init (GDataGDPostalAddress *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GDATA_TYPE_GD_POSTAL_ADDRESS, GDataGDPostalAddressPrivate);
}

static void
gdata_gd_postal_address_finalize (GObject *object)
{
	GDataGDPostalAddressPrivate *priv = GDATA_GD_POSTAL_ADDRESS (object)->priv;

	g_free (priv->address);
	g_free (priv->relation_type);
	g_free (priv->label);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (gdata_gd_postal_address_parent_class)->finalize (object);
}

static void
gdata_gd_postal_address_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GDataGDPostalAddressPrivate *priv = GDATA_GD_POSTAL_ADDRESS (object)->priv;

	switch (property_id) {
		case PROP_ADDRESS:
			g_value_set_string (value, priv->address);
			break;
		case PROP_RELATION_TYPE:
			g_value_set_string (value, priv->relation_type);
			break;
		case PROP_LABEL:
			g_value_set_string (value, priv->label);
			break;
		case PROP_IS_PRIMARY:
			g_value_set_boolean (value, priv->is_primary);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
gdata_gd_postal_address_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GDataGDPostalAddress *self = GDATA_GD_POSTAL_ADDRESS (object);

	switch (property_id) {
		case PROP_ADDRESS:
			gdata_gd_postal_address_set_address (self, g_value_get_string (value));
			break;
		case PROP_RELATION_TYPE:
			gdata_gd_postal_address_set_relation_type (self, g_value_get_string (value));
			break;
		case PROP_LABEL:
			gdata_gd_postal_address_set_label (self, g_value_get_string (value));
			break;
		case PROP_IS_PRIMARY:
			gdata_gd_postal_address_set_is_primary (self, g_value_get_boolean (value));
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static gboolean
pre_parse_xml (GDataParsable *parsable, xmlDoc *doc, xmlNode *root_node, gpointer user_data, GError **error)
{
	xmlChar *address, *rel, *label, *primary;
	gboolean primary_bool;
	GDataGDPostalAddressPrivate *priv = GDATA_GD_POSTAL_ADDRESS (parsable)->priv;

	address = xmlNodeListGetString (doc, root_node->children, TRUE);
	if (address == NULL || *address == '\0')
		return gdata_parser_error_required_content_missing (root_node, error);

	rel = xmlGetProp (root_node, (xmlChar*) "rel");
	if (rel != NULL && *rel == '\0') {
		xmlFree (address);
		return gdata_parser_error_required_property_missing (root_node, "rel", error);
	}

	/* Is it the primary postal address? */
	primary = xmlGetProp (root_node, (xmlChar*) "primary");
	if (primary == NULL || xmlStrcmp (primary, (xmlChar*) "false") == 0)
		primary_bool = FALSE;
	else if (xmlStrcmp (primary, (xmlChar*) "true") == 0)
		primary_bool = TRUE;
	else {
		gdata_parser_error_unknown_property_value (root_node, "primary", (gchar*) primary, error);
		xmlFree (primary);
		xmlFree (address);
		xmlFree (rel);
		return FALSE;
	}
	xmlFree (primary);

	/* Other properties */
	label = xmlGetProp (root_node, (xmlChar*) "label");

	priv->address = g_strdup ((gchar*) address);
	priv->relation_type = g_strdup ((gchar*) rel);
	priv->label = g_strdup ((gchar*) label);
	priv->is_primary = primary_bool;

	xmlFree (address);
	xmlFree (rel);
	xmlFree (label);

	return TRUE;
}

static gboolean
parse_xml (GDataParsable *parsable, xmlDoc *doc, xmlNode *node, gpointer user_data, GError **error)
{
	/* Textual content's handled in pre_parse_xml */
	if (node->type != XML_ELEMENT_NODE)
		return TRUE;

	if (GDATA_PARSABLE_CLASS (gdata_gd_postal_address_parent_class)->parse_xml (parsable, doc, node, user_data, error) == FALSE) {
		/* Error! */
		return FALSE;
	}

	return TRUE;
}

static void
pre_get_xml (GDataParsable *parsable, GString *xml_string)
{
	GDataGDPostalAddressPrivate *priv = GDATA_GD_POSTAL_ADDRESS (parsable)->priv;

	if (priv->relation_type != NULL)
		g_string_append_printf (xml_string, " rel='%s'", priv->relation_type);
	if (priv->label != NULL)
		g_string_append_printf (xml_string, " label='%s'", priv->label);

	if (priv->is_primary == TRUE)
		g_string_append (xml_string, " primary='true'");
	else
		g_string_append (xml_string, " primary='false'");
}

static void
get_xml (GDataParsable *parsable, GString *xml_string)
{
	GDataGDPostalAddressPrivate *priv = GDATA_GD_POSTAL_ADDRESS (parsable)->priv;

	g_string_append (xml_string, priv->address);
}

static void
get_namespaces (GDataParsable *parsable, GHashTable *namespaces)
{
	g_hash_table_insert (namespaces, (gchar*) "gd", (gchar*) "http://schemas.google.com/g/2005");
}

/**
 * gdata_gd_postal_address_new:
 * @address: the postal address, in human-readable format (new lines are significant)
 * @relation_type: the relationship between the address and its owner, or %NULL
 * @label: a human-readable label for the address, or %NULL
 * @is_primary: %TRUE if this phone number is its owner's primary number, %FALSE otherwise
 *
 * Creates a new #GDataGDPostalAddress. More information is available in the <ulink type="http"
 * url="http://code.google.com/apis/gdata/docs/1.0/elements.html#gdPostalAddress">GData specification</ulink>.
 *
 * Return value: a new #GDataGDPostalAddress, or %NULL; unref with g_object_unref()
 *
 * Since: 0.2.0
 **/
GDataGDPostalAddress *
gdata_gd_postal_address_new (const gchar *address, const gchar *relation_type, const gchar *label, gboolean is_primary)
{
	g_return_val_if_fail (address != NULL && *address != '\0', NULL);
	g_return_val_if_fail (relation_type == NULL || *relation_type != '\0', NULL);
	return g_object_new (GDATA_TYPE_GD_POSTAL_ADDRESS, "address", address, "relation-type", relation_type,
			     "label", label, "is-primary", is_primary, NULL);
}

/**
 * gdata_gd_postal_address_compare:
 * @a: a #GDataGDPostalAddress, or %NULL
 * @b: another #GDataGDPostalAddress, or %NULL
 *
 * Compares the two postal addresses in a strcmp() fashion. %NULL values are handled gracefully, with
 * %0 returned if both @a and @b are %NULL, %-1 if @a is %NULL and %1 if @b is %NULL.
 *
 * The comparison of non-%NULL values is done on the basis of the @address property of the #GDataGDPostalAddress<!-- -->es.
 *
 * Return value: %0 if @a equals @b, %-1 or %1 as appropriate otherwise
 *
 * Since: 0.4.0
 **/
gint
gdata_gd_postal_address_compare (const GDataGDPostalAddress *a, const GDataGDPostalAddress *b)
{
	if (a == NULL && b != NULL)
		return -1;
	else if (b == NULL)
		return 1;

	if (a == b)
		return 0;
	return g_strcmp0 (a->priv->address, b->priv->address);
}

/**
 * gdata_gd_postal_address_get_address:
 * @self: a #GDataGDPostalAddress
 *
 * Gets the #GDataGDPostalAddress:address property.
 *
 * Return value: the postal address itself, or %NULL
 *
 * Since: 0.4.0
 **/
const gchar *
gdata_gd_postal_address_get_address (GDataGDPostalAddress *self)
{
	g_return_val_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self), NULL);
	return self->priv->address;
}

/**
 * gdata_gd_postal_address_set_address:
 * @self: a #GDataGDPostalAddress
 * @address: the new postal address
 *
 * Sets the #GDataGDPostalAddress:address property to @address.
 *
 * Since: 0.4.0
 **/
void
gdata_gd_postal_address_set_address (GDataGDPostalAddress *self, const gchar *address)
{
	g_return_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self));
	g_return_if_fail (address != NULL && *address != '\0');

	g_free (self->priv->address);
	self->priv->address = g_strdup (address);
	g_object_notify (G_OBJECT (self), "address");
}

/**
 * gdata_gd_postal_address_get_relation_type:
 * @self: a #GDataGDPostalAddress
 *
 * Gets the #GDataGDPostalAddress:relation-type property.
 *
 * Return value: the postal address' relation type, or %NULL
 *
 * Since: 0.4.0
 **/
const gchar *
gdata_gd_postal_address_get_relation_type (GDataGDPostalAddress *self)
{
	g_return_val_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self), NULL);
	return self->priv->relation_type;
}

/**
 * gdata_gd_postal_address_set_relation_type:
 * @self: a #GDataGDPostalAddress
 * @relation_type: the new relation type for the postal_address, or %NULL
 *
 * Sets the #GDataGDPostalAddress:relation-type property to @relation_type.
 *
 * Set @relation_type to %NULL to unset the property in the postal address.
 *
 * Since: 0.4.0
 **/
void
gdata_gd_postal_address_set_relation_type (GDataGDPostalAddress *self, const gchar *relation_type)
{
	g_return_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self));
	g_return_if_fail (relation_type == NULL || *relation_type != '\0');

	g_free (self->priv->relation_type);
	self->priv->relation_type = g_strdup (relation_type);
	g_object_notify (G_OBJECT (self), "relation-type");
}

/**
 * gdata_gd_postal_address_get_label:
 * @self: a #GDataGDPostalAddress
 *
 * Gets the #GDataGDPostalAddress:label property.
 *
 * Return value: the postal address' label, or %NULL
 *
 * Since: 0.4.0
 **/
const gchar *
gdata_gd_postal_address_get_label (GDataGDPostalAddress *self)
{
	g_return_val_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self), NULL);
	return self->priv->label;
}

/**
 * gdata_gd_postal_address_set_label:
 * @self: a #GDataGDPostalAddress
 * @label: the new label for the postal address, or %NULL
 *
 * Sets the #GDataGDPostalAddress:label property to @label.
 *
 * Set @label to %NULL to unset the property in the postal address.
 *
 * Since: 0.4.0
 **/
void
gdata_gd_postal_address_set_label (GDataGDPostalAddress *self, const gchar *label)
{
	g_return_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self));

	g_free (self->priv->label);
	self->priv->label = g_strdup (label);
	g_object_notify (G_OBJECT (self), "label");
}

/**
 * gdata_gd_postal_address_is_primary:
 * @self: a #GDataGDPostalAddress
 *
 * Gets the #GDataGDPostalAddress:is-primary property.
 *
 * Return value: %TRUE if this is the primary postal address, %FALSE otherwise
 *
 * Since: 0.4.0
 **/
gboolean
gdata_gd_postal_address_is_primary (GDataGDPostalAddress *self)
{
	g_return_val_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self), FALSE);
	return self->priv->is_primary;
}

/**
 * gdata_gd_postal_address_set_is_primary:
 * @self: a #GDataGDPostalAddress
 * @is_primary: %TRUE if this is the primary postal address, %FALSE otherwise
 *
 * Sets the #GDataGDPostalAddress:is-primary property to @is_primary.
 *
 * Since: 0.4.0
 **/
void
gdata_gd_postal_address_set_is_primary (GDataGDPostalAddress *self, gboolean is_primary)
{
	g_return_if_fail (GDATA_IS_GD_POSTAL_ADDRESS (self));

	self->priv->is_primary = is_primary;
	g_object_notify (G_OBJECT (self), "is-primary");
}
