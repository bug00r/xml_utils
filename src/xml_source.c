#include "xml_source.h"

static XmlSource* xml_source_new(XmlSourceType type, ResourceFile *res_file) {
    
    XmlSource* newxml_source = NULL;

    XmlSource _tmp_newxml_source = { type, &res_file->file_size, res_file->data, { res_file } };

    newxml_source = malloc(sizeof(XmlSource));
    
    memcpy(newxml_source, &_tmp_newxml_source, sizeof(XmlSource));
    
    return newxml_source;
}

static XmlSource* _xml_source_from_res_search(ArchiveResource* ar, const char *searchname) {
    
    XmlSource *result = NULL;

    ResourceSearchResult* searchresult = archive_resource_search_by_name(ar, (const unsigned char *)searchname);

    if ( searchresult->cnt == 1 ) {
        result = xml_source_new(RESOURCE_FILE, searchresult->files[0]);
    }

    resource_search_result_free(&searchresult);

    return result;
}

XmlSource* xml_source_from_resname(ArchiveResource* ar, const char *name) {
    
    XmlSource *result = NULL;

    if (ar != NULL && name != NULL) {
        char * searchname = format_string_new("xml/%s.xml", name);
        result = _xml_source_from_res_search(ar, searchname);
        free(searchname);
    }

    return result;
}

XmlSource* xml_source_from_resname_full(ArchiveResource* ar, const char *path, const char *name, const char *suffix) {
    XmlSource *result = NULL;

    if (ar != NULL && name != NULL && path != NULL && suffix != NULL) {
        char * searchname = format_string_new("%s%s.%s", path, name, suffix);

        printf("search xsl: %s\n", searchname);

        result = _xml_source_from_res_search(ar, searchname);
        
        free(searchname);
    }

    return result;
}

XmlSource* xml_source_from_resfile(ResourceFile *resfile) {

    XmlSource *result = NULL;

    if (resfile) {
        result = xml_source_new(RESOURCE_FILE, resfile);
    }

    return result;

}

void xml_source_free(XmlSource **source) {

    if( source != NULL && *source != NULL ) {
    
        XmlSource *_delete_source = *source;
    
        switch(_delete_source->type) {
            case RESOURCE_FILE: resource_file_free((ResourceFile**)&_delete_source->data.resfile);
                                break;
        }
    
        free(_delete_source);
    
        *source = NULL;
    }
}
