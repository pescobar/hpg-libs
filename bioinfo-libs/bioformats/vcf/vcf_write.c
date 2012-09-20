#include "vcf_write.h"

int write_vcf_file(vcf_file_t *vcf_file, FILE *fd) {
    if(vcf_file == NULL) {
        return -1;
    }
    if (fd == NULL) {
        return -2;
    }
    
    // Write header: file format, header entries and delimiter
    write_vcf_header(vcf_file, fd);
    // Write records: grouped in batches
    vcf_batch_t *batch;
    while ((batch = fetch_vcf_batch(vcf_file)) != NULL) {
        write_vcf_batch(batch, fd);
    }
    
    return 0;
}

void write_vcf_header(vcf_file_t* vcf_file, FILE* fd) {
    if(vcf_file == NULL || fd == NULL) {
        return;
    }

    // Write fileformat
    write_vcf_fileformat(vcf_file, fd);
    // Write header entries
    for (int i = 0; i < vcf_file->header_entries->size; i++) {
        write_vcf_header_entry((vcf_header_entry_t*) array_list_get(i, vcf_file->header_entries), fd);
    }
    // Write delimiter
    write_vcf_delimiter(vcf_file, fd);
}


void write_vcf_fileformat(vcf_file_t *vcf_file, FILE *fd) {
    if(vcf_file == NULL || vcf_file->format == NULL || fd == NULL) {
        return;
    }
    
    fprintf(fd, "##fileformat=%s\n", vcf_file->format);
}

void write_vcf_header_entry(vcf_header_entry_t *entry, FILE *fd) {
    if (entry == NULL || fd == NULL) {
        return;
    }

    fprintf(fd, "##");

    // Entries with the form ##value
    if (entry->name == NULL) {
        char *value = entry->values->first_p->data_p; // Just first (and only) value
        fprintf(fd, "%s\n", value);
        return;
    }

    fprintf(fd, "%s", entry->name);
    // Entries with the form ##name=value
    if (entry->num_keys == 0 && entry->num_values > 0) {
        char *value = entry->values->first_p->data_p; // Just first (and only) value
        fprintf(fd, "=%s\n", value);
    }
    // Entries with the form ##name=<field_id=value,field_id=value,...>
    else if (entry->num_keys > 0 && entry->num_keys == entry->num_values) {
        fprintf(fd, "=<");
        list_item_t *key = entry->keys->first_p;
        list_item_t *value = entry->values->first_p;
        
        if (strcmp("Description", (char*) key->data_p) != 0)
        {
            fprintf(fd, "%s=%s", (char*) key->data_p, (char*) value->data_p);
        } else 
        {
            fprintf(fd, "%s=\"%s\"", (char*) key->data_p, (char*) value->data_p);
        }
        
        // Get next pair key-value
        key = key->next_p;
        value = value->next_p;
        
        while (key != NULL && value != NULL) {
            if (strcmp("Description", (char*) key->data_p) != 0) {
                fprintf(fd, ",%s=%s", (char*) key->data_p, (char*) value->data_p);
            } else {
                fprintf(fd, ",%s=\"%s\"", (char*) key->data_p, (char*) value->data_p);
            }
            
            // Get next pair key-value
            key = key->next_p;
            value = value->next_p;
        } 
        
        fprintf(fd, ">\n");
    }
}

void write_vcf_delimiter(vcf_file_t *vcf_file, FILE *fd) {
    if(vcf_file == NULL || fd == NULL) {
        return;
    }
    
    fprintf(fd, "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT");

    for (int i = 0; i < vcf_file->samples_names->size; i++) {
        fprintf(fd, "\t%s", (char*) vcf_file->samples_names->items[i]);
    }
       
    fprintf(fd, "\n");
}

void write_vcf_batch(vcf_batch_t *vcf_batch, FILE *fd) {
    if(vcf_batch == NULL || fd == NULL) {
        return;
    }
    
    vcf_record_t *record;
    for (int i = 0; i < vcf_batch->records->size; i++) {
//         record = array_list_get(i, vcf_batch->records);
        record = vcf_batch->records->items[i];
        write_vcf_record(record, fd);
    }
}

void write_vcf_record(vcf_record_t* vcf_record, FILE *fd) {
    if(vcf_record == NULL || fd == NULL) {
        return;
    }
    
    fprintf(fd, "%s\t%ld\t%s\t%s\t%s\t", vcf_record->chromosome, vcf_record->position, vcf_record->id, vcf_record->reference, vcf_record->alternate);
    if (vcf_record->quality < 0) {
        fprintf(fd, ".\t");
    } else {
        fprintf(fd, "%.2f\t", vcf_record->quality);
    }
    fprintf(fd, "%s\t%s\t%s", vcf_record->filter, vcf_record->info, vcf_record->format);

    for (int i = 0; i < vcf_record->samples->size; i++) {
        fprintf(fd, "\t%s", (char*) array_list_get(i, vcf_record->samples));
    }

    fprintf(fd, "\n");
}