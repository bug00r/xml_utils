#ifndef XML_SOURCE_H
#define XML_SOURCE_H

#if 0
    Concept global, just for remembering target: 
        - Utils have to work with xml sources. So we need XmlSource with different loading interfaces. At first from memory or memory Wrapper Pattern
          in this case from resource too.        
#endif

#include "resource.h"

typedef enum {
    RESOURCE_FILE
} XmlSourceType;

typedef struct {
    const XmlSourceType     type;
    const size_t			  * const src_size; /* size of xml source in byte */
	const unsigned char 	  * const src_data; /* data of xml source as byte array */
    union {
        const ResourceFile * const resfile;
    } data;
} XmlSource;

/*
	This function reads an xml source from fix archiv folder xml.
	
    Example:
        This will search at xml in archiv "xml/talents.xml"
        XmlSource *result = xml_source_from_resname(archiv, "talents");

        This will search at xml in archiv "xml/breeds.xml"
        XmlSource *result = xml_source_from_resname(archiv, "breeds");

        Invalid use:
        This will search at xml in archiv "xml/xml2/breeds.xml"
        XmlSource *result = xml_source_from_resname(archiv, "xml2/breeds");

	Parameter			Decription
	---------			-----------------------------------------
	ar		            resource archive for search within.
	name				name of the resource, without path or suffix.
	
	returns: new xml source object or NULL if no resource was found by name
	
*/
XmlSource* xml_source_from_resname(ArchiveResource* ar, const char *name);
XmlSource* xml_source_from_resname_full(ArchiveResource* ar, const char *path, const char *name, const char *suffix);

/*
	This function reads an xml source from resource_file object.

	Parameter			Decription
	---------			-----------------------------------------
	resfile		        resource file object
	
	returns: new xml source object or NULL if no resource was found by name
	
*/
XmlSource* xml_source_from_resfile(ResourceFile *resfile);

/*
	The function "xml_source_free" frees the memory of xml source complete.
	
	Parameter			Decription
	---------			-----------------------------------------
	source				pointer to pointer of source, this pointer will be NULL
	
*/
void xml_source_free(XmlSource **source);

#endif