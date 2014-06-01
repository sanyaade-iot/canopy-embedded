/*
 * Copyright 2014 - Greg Prisament
 */
#include "sddl.h"

struct SDDLDocument_t
{
    unsigned numAuthors;
    char **authors;
    char *description;
    unsigned numProperties;
    SDDLProperty *properties;
};

struct SDDLProperty
{
    char *name;
    SDDLPropertyTypeEnum type;
    char *description;
};

struct SDDLControl
{
    SDDLProperty base;
    SDDLControlTypeEnum controlType;
    double *maxValue;
    double *minValue;
    SDDLNumericDisplayHintEnum numericDisplayHint;
    char *regex;
    char *units;
};

struct SDDLSensor
{
    SDDLProperty base;
    double *maxValue;
    double *minValue;
    SDDLNumericDisplayHintEnum numericDisplayHint;
    char *regex;
    char *units;
};

struct SDDLClass_t
{
    SDDLProperty base;
    unsigned numAuthors;
    char **authors;
    char *description;
    unsigned numProperties;
    SDDLProperty *properties;
};

static SDDLNumericDisplayHintEnum _display_hint_from_string(const char *sz)
{
    if (!strcmp(sz, "normal"))
    {
        return SDDL_NUMERIC_DISPLAY_HINT_NORMAL;
    }
    else if (!strcmp(sz, "percentage"))
    {
        return SDDL_NUMERIC_DISPLAY_HINT_PERCENTAGE;
    }
    else if (!strcmp(sz, "hex"))
    {
        return SDDL_NUMERIC_DISPLAY_HINT_HEX;
    }
    else if (!strcmp(sz, "scientific"))
    {
        return SDDL_NUMERIC_DISPLAY_HINT_SCIENTIFIC;
    }
    return SDDL_NUMERIC_DISPLAY_HINT_INVALID;
}

static SDDLControlTypeEnum _control_type_from_string(const char *sz)
{
    if (!strcmp(sz, "parameter"))
    {
        return SDDL_CONTROL_TYPE_PARAMETER;
    }
    else if (!strcmp(sz, "trigger"))
    {
        return SDDL_CONTROL_TYPE_TRIGGER;
    }
    return SDDL_CONTROL_TYPE_INVALID;
}

static SDDLControl _sddl_parse_control(RedString decl, RedJsonObj def)
{
    SDDLControl out;

    out = calloc(1, sizeof(struct SDDLControl_t));
    if (!out)
    {
        return NULL;
    }

    out->prop.type = SDDL_PROPERTY_TYPE_CONTROL;
    /* TODO: set defaults */

    numKeys = RedJsonObject_NumItems(jsonObj);
    keysArray = RedJsonObject_NewKeysArray(jsonObj);

    for (i = 0; i < numKeys; i++)
    {
        RedJsonValue val = RedJsonObject_Get(jsonObj, keysArray[i]);
        RedString key = RedString_New(keysArray[i]);

        if (RedString_Equals(key, "control-type"))
        {
            char *controlTypeString;
            if (!RedJsonValue_IsString(val))
            {
                printf("control-type must be string");
                return NULL;
            }
            controlTypeString = RedJsonValue_GetString(val);
            out->controlType = _control_type_from_string(controlTypeString);
            if (out->controlType == SDDL_CONTROL_TYPE_INVALID)
            {
                printf("invalid control-type %s", controlTypeString);
                return NULL;
            }
        }
        else if (RedString_Equals(key, "datatype"))
        {
            char *datatypeString;
            if (!RedJsonValue_IsString(val))
            {
                printf("datatype must be string");
                return NULL;
            }
            datatypeString = RedJsonValue_GetString(val);
            out->datatype = _datatype_from_string(datatypeString);
            if (out->datatype == SDDL_DATATYPE_INVALID)
            {
                printf("invalid datatype %s", controlTypeString);
                return NULL;
            }
        }
        else if (RedString_Equals(key, "description"))
        {
            char *description;
            if (!RedJsonValue_IsString(val))
            {
                printf("description must be string");
                return NULL;
            }
            description = RedJsonValue_GetString(val);
            out->prop.description = RedString_strdup(description);
            if (!out->prop.description)
            {
                printf("OOM duplicating description string");
                return NULL;
            }
        }
        else if (RedString_Equals(key, "max-value"))
        {
            if (RedJsonValue_IsNull(val))
            {
                out->prop.maxValue = NULL;
            }
            else
            {
                double * pMaxValue;
                if (!RedJsonValue_IsNumber(val))
                {
                    printf("max-value must be number or null");
                    return NULL;
                }
                pMaxValue = malloc(sizeof(double));
                if (!pMaxValue)
                {
                    printf("OOM allocating max-value");
                    return NULL;
                }
                *pMaxValue = RedJsonValue_GetNumber(val);
                out->maxValue = &pMaxValue;
            }
        }
        else if (RedString_Equals(key, "min-value"))
        {
            if (RedJsonValue_IsNull(val))
            {
                out->prop.minValue = NULL;
            }
            else
            {
                double * pMaxValue;
                if (!RedJsonValue_IsNumber(val))
                {
                    printf("min-value must be number or null");
                    return NULL;
                }
                pMinValue = malloc(sizeof(double));
                if (!pMinValue)
                {
                    printf("OOM allocating min-value");
                    return NULL;
                }
                *pMinValue = RedJsonValue_GetNumber(val);
                out->minValue = &pMinValue;
            }
        }
        else if (RedString_Equals(key, "numeric-display-hint"))
        {
            char *displayHintString;
            if (!RedJsonValue_IsString(val))
            {
                printf("datatype must be string");
                return NULL;
            }
            displayHintString = RedJsonValue_GetString(val);
            out->numericDisplayHint = _display_hint_from_string(displayHintString);
            if (out->numericDisplayHint == SDDL_DISPLAY_HINT_INVALID)
            {
                printf("invalid numeric-display-hint %s", displayHintString);
                return NULL;
            }
        }
        else if (RedString_Equals(key, "regex"))
        {
            char *regex;
            if (!RedJsonValue_IsString(val))
            {
                printf("regex must be string");
                return NULL;
            }
            regex = RedJsonValue_GetString(val);
            out->prop.regex = RedString_strdup(regex);
            if (!out->prop.regex)
            {
                printf("OOM duplicating regex string");
                return NULL;
            }
        }
        else if (RedString_Equals(key, "units"))
        {
            char *units;
            if (!RedJsonValue_IsString(val))
            {
                printf("units must be string");
                return NULL;
            }
            units = RedJsonValue_GetString(val);
            out->prop.units = RedString_strdup(units);
            if (!out->prop.units)
            {
                printf("OOM duplicating units string");
                return NULL;
            }
        }
        else
        {
            printf("Unexpected field: %s", key);
        }
        RedString_Free(key);
    }

    return out;
}

static SDDLSensor _sddl_parse_sensor(RedString decl, RedJsonObj def)
{
    SDDLSensor out;

    out = calloc(1, sizeof(struct SDDLSensor_t));
    if (!out)
    {
        return NULL;
    }

    RedStringList split = RedString_Split(key, ' ');
    RedString name = RedStringList_GetString(split, 1);
    RedStringList_Free(split);

    out->prop.type = SDDL_PROPERTY_TYPE_SENSOR;
    /* TODO: set defaults */

    numKeys = RedJsonObject_NumItems(jsonObj);
    keysArray = RedJsonObject_NewKeysArray(jsonObj);

    for (i = 0; i < numKeys; i++)
    {
        RedJsonValue val = RedJsonObject_Get(jsonObj, keysArray[i]);
        RedString key = RedString_New(keysArray[i]);

        if (RedString_Equals(key, "datatype"))
        {
            char *datatypeString;
            if (!RedJsonValue_IsString(val))
            {
                printf("datatype must be string");
                return NULL;
            }
            datatypeString = RedJsonValue_GetString(val);
            out->datatype = _datatype_from_string(datatypeString);
            if (out->datatype == SDDL_DATATYPE_INVALID)
            {
                printf("invalid datatype %s", controlTypeString);
                return NULL;
            }
        }
        else if (RedString_Equals(key, "description"))
        {
            char *description;
            if (!RedJsonValue_IsString(val))
            {
                printf("description must be string");
                return NULL;
            }
            description = RedJsonValue_GetString(val);
            out->prop.description = RedString_strdup(description);
            if (!out->prop.description)
            {
                printf("OOM duplicating description string");
                return NULL;
            }
        }
        else if (RedString_Equals(key, "max-value"))
        {
            if (RedJsonValue_IsNull(val))
            {
                out->prop.maxValue = NULL;
            }
            else
            {
                double * pMaxValue;
                if (!RedJsonValue_IsNumber(val))
                {
                    printf("max-value must be number or null");
                    return NULL;
                }
                pMaxValue = malloc(sizeof(double));
                if (!pMaxValue)
                {
                    printf("OOM allocating max-value");
                    return NULL;
                }
                *pMaxValue = RedJsonValue_GetNumber(val);
                out->maxValue = &pMaxValue;
            }
        }
        else if (RedString_Equals(key, "min-value"))
        {
            if (RedJsonValue_IsNull(val))
            {
                out->prop.minValue = NULL;
            }
            else
            {
                double * pMaxValue;
                if (!RedJsonValue_IsNumber(val))
                {
                    printf("min-value must be number or null");
                    return NULL;
                }
                pMinValue = malloc(sizeof(double));
                if (!pMinValue)
                {
                    printf("OOM allocating min-value");
                    return NULL;
                }
                *pMinValue = RedJsonValue_GetNumber(val);
                out->minValue = &pMinValue;
            }
        }
        else if (RedString_Equals(key, "numeric-display-hint"))
        {
            char *displayHintString;
            if (!RedJsonValue_IsString(val))
            {
                printf("datatype must be string");
                return NULL;
            }
            displayHintString = RedJsonValue_GetString(val);
            out->numericDisplayHint = _display_hint_from_string(displayHintString);
            if (out->numericDisplayHint == SDDL_DISPLAY_HINT_INVALID)
            {
                printf("invalid numeric-display-hint %s", displayHintString);
                return NULL;
            }
        }
        else if (RedString_Equals(key, "regex"))
        {
            char *regex;
            if (!RedJsonValue_IsString(val))
            {
                printf("regex must be string");
                return NULL;
            }
            regex = RedJsonValue_GetString(val);
            out->prop.regex = RedString_strdup(regex);
            if (!out->prop.regex)
            {
                printf("OOM duplicating regex string");
                return NULL;
            }
        }
        else if (RedString_Equals(key, "units"))
        {
            char *units;
            if (!RedJsonValue_IsString(val))
            {
                printf("units must be string");
                return NULL;
            }
            units = RedJsonValue_GetString(val);
            out->prop.units = RedString_strdup(units);
            if (!out->prop.units)
            {
                printf("OOM duplicating units string");
                return NULL;
            }
        }
        else
        {
            printf("Unexpected field: %s", key);
        }
        RedString_Free(key);
    }
    
    return out;
}

static SDDLClass _sddl_parse_class(RedString decl, RedJsonObj def)
{
}

SDDLDocument sddl_parse(CanopyContext canopy, const char *sddl)
{
    RedJsonObject jsonObj;

    jsonObj = RedJson_Parse(sddl);

    numKeys = RedJsonObject_NumItems(jsonObj);
    keysArray = RedJsonObject_NewKeysArray(jsonObj);
    
    for (i = 0; i < numKeys; i++)
    {
        RedJsonValue val = RedJsonObject_Get(jsonObj, keysArray[i]);
        RedString key = RedString_New(keysArray[i]);
        if (RedString_BeginsWith(key, "control "))
        {
            if (!RedJsonValue_IsObject())
            {
                printf("Expected object for control definition");
                return NULL;
            }
            RedJsonObject obj = RedJsonValue_GetObject(obj);
            SDDLControl control = _sddl_parse_control(key, obj);
            if (!control)
            {
                return NULL;
            }
            doc->numProperties++;
            doc->properties = realloc(doc->numProperties, sizeof(SDDLProperty));
            if (!doc->properties)
            {
                printf("OOM expanding doc->properties");
                return NULL;
            }
            doc->properties[doc->numProperties - 1] = SDDL_PROPERTY(control);
        }
        else if  (RedString_BeginsWith(key, "sensor "))
        {
            if (!RedJsonValue_IsObject())
            {
                printf("Expected object for sensor definition");
                return NULL;
            }
            RedJsonObject obj = RedJsonValue_GetObject(obj);
            SDDLSensor sensor = _sddl_parse_sensor(key, obj);
            if (!sensor)
            {
                return NULL;
            }
            doc->numProperties++;
            doc->properties = realloc(doc->numProperties, sizeof(SDDLProperty));
            if (!doc->properties)
            {
                printf("OOM expanding doc->properties");
                return NULL;
            }
            doc->properties[doc->numProperties - 1] = SDDL_PROPERTY(sensor);
        }
        else if  (RedString_BeginsWith(key, "class "))
        {
            if (!RedJsonValue_IsObject())
            {
                printf("Expected object for class definition");
                return NULL;
            }
            RedJsonObject obj = RedJsonValue_GetObject(obj);
            SDDLClass class = _sddl_parse_class(key, obj);
            if (!class)
            {
                return NULL;
            }
            doc->numProperties++;
            doc->properties = realloc(doc->numProperties, sizeof(SDDLProperty));
            if (!doc->properties)
            {
                printf("OOM expanding doc->properties");
                return NULL;
            }
            doc->properties[doc->numProperties - 1] = SDDL_PROPERTY(class);
        }
        else if (RedString_Equals(key, "authors"))
        {
            if (!RedJsonValue_IsArray(val))
            {
                printf("Expected list for authors");
                return NULL;
            }
            
            RedJsonArray authorList = RedJsonValue_GetArray(val);
            doc->numAuthors = RedJsonArray_NumItems(authorList);
            doc->authors = calloc(doc->numAuthors, sizeof(char *));
            if (!doc->authors)
            {
                printf("OOM - Failed to allocate author list");
                return NULL;
            }
            for (i = 0; i < doc->numAuthors; i++)
            {
                if (!RedJsonArray_IsEntryString(authorList, i))
                {
                    printf("Expected string for authors list entry");
                    return NULL;
                }
            }
        }
        else if (RedString_Equals(key, "description"))
        {
            char *description;
            if (!RedJsonValue_IsString(val))
            {
                printf("description must be string");
                return NULL;
            }
            description = RedJsonValue_GetString(val);
            doc->description = RedString_strdup(description);
            if (!doc->description)
            {
                printf("OOM duplicating description string");
                return NULL;
            }
        }

        RedString_Free(key);
    }

    return doc;
}

unsigned sddl_document_num_authors(SDDLDocument doc)
{
    return doc->numAuthors;
}

const char * sddl_document_author(SDDLDocument doc, unsigned index)
{
    return doc->authors[index];
}

unsigned sddl_document_num_properties(SDDLDocument doc)
{
    return doc->numProperties;
}

SDDLProperty sddl_document_property(SDDLDocument doc, unsigned index) 
{
    return doc->properties[index]
}

bool sddl_is_control(SDDLProperty prop)
{
    return (prop->type == SDDL_PROPERTY_TYPE_CONTROL);
}
bool sddl_is_sensor(SDDLProperty prop)
{
    return (prop->type == SDDL_PROPERTY_TYPE_SENSOR);
}
bool sddl_is_class(SDDLProperty prop)
{
    return (prop->type == SDDL_PROPERTY_TYPE_CLASS);
}

SDDLControl SDDL_CONTROL(SDDProperty prop)
{
    assert(sddl_is_control(prop));
    return (SDDLControl)prop;
}
SDDLSensor SDDL_SENSOR(SDDProperty prop)
{
    assert(sddl_is_sensor(prop));
    return (SDDLSensor)prop;
}
SDDLClass SDDL_CLASS(SDDProperty prop)
{
    assert(sddl_is_class(prop));
    return (SDDLClass)prop;
}

const char * sddl_control_name(SDDLControl control)
{
    return control->prop.name;
}
SDDLControlTypeEnum sddl_control_type(SDDLControl control)
{
    return control->controlType;
}
SDDLDatatypeEnum sddl_control_datatype(SDDLControl control)
{
    return control->datatype;
}
const char * sddl_control_description(SDDLControl control)
{
    return control->prop.description;
}
const double * sddl_control_max_value(SDDLControl control)
{
    return control->maxValue;
}
const double * sddl_control_min_value(SDDLControl control)
{
    return control->minValue;
}
SDDLNumericDisplayHintEnum sddl_control_numeric_display_hint(SDDLControl control)
{
    return control->numericDisplayHint;
}
const char * sddl_control_regex(SDDLControl control)
{
    return control->regex;
}
const char * sddl_control_units(SDDLControl control)
{
    return control->units;
}

const char * sddl_sensor_name(SDDLSensor sensor)
{
    return sensor->prop.name;
}
SDDLSensorTypeEnum sddl_sensor_type(SDDLSensor sensor)
{
    return sensor->sensorType;
}
SDDLDatatypeEnum sddl_sensor_datatype(SDDLSensor sensor)
{
    return sensor->datatype;
}
const char * sddl_sensor_description(SDDLSensor sensor)
{
    return sensor->prop.description;
}
const double * sddl_sensor_max_value(SDDLSensor sensor)
{
    return sensor->maxValue;
}
const double * sddl_sensor_min_value(SDDLSensor sensor)
{
    return sensor->minValue;
}
SDDLNumericDisplayHintEnum sddl_sensor_numeric_display_hint(SDDLSensor sensor)
{
    return sensor->numericDisplayHint;
}
const char * sddl_sensor_regex(SDDLSensor sensor)
{
    return sensor->regex;
}
const char * sddl_sensor_units(SDDLSensor sensor)
{
    return sensor->units;
}

unsigned sddl_class_num_authors(SDDLClass cls)
{
    return cls->numAuthors;
}
const char * sddl_class_author(SDDLClass cls, unsigned index)
{
    return cls->authors[index];
}
const char * sddl_class_description(SDDLClass cls)
{
    return cls->prop.description;
}
unsigned sddl_class_num_properties(SDDLClass cls)
{
    return cls->numProperties;
}
SDDLProperty sddl_class_property(SDDLClass cls, unsigned index) 
{
    return cls->properties[index]
}
