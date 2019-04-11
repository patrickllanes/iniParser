/*
 ============================================================================
 Name        : iniParser.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LENGTH(x)	(int)sizeof(x)/sizeof(x[0])
#define INI_FILE_PATH 	"./Test.ini"
#define	SECTION			'['
#define COMMENT			';'
#define CR				'\r'

char *valid_sections[] = {"Information", "Event_1", "Event_2"};
char *valid_info_tags[] = {"ID", "Location_Name", "Time_Zone"};
char *valid_event1_tags[] = {"Low_Threshold_In_Percent", "High_Threshold_In_Percent", "Hysteresis_In_Percent"};
char *valid_event2_tags[] = {"Low_Threshold_In_Percent", "High_Threshold_In_Percent", "Hysteresis_In_Percent"};

struct tagValue {
	char tag[50];
	char value[50];
};

struct section {
	int tag_count;
	struct tagValue *tag_value;
};

int getFileSize(FILE* fp) {
	int file_size;

	// Seek to end of file to get file size
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);

	// Return to beginning of file
	fseek(fp, 0, 0);

	return file_size;
}

int getSection(char *line) {
	int i;

	// Assumption: each section line ends with line feed character \n
	// Based on this assumption, I can compare valid sections by comparing
	// line pointer + 1 to emit first bracket up to length of line - 3 to
	// remove last bracket and new line character
	for (i = 0; i < ARRAY_LENGTH(valid_sections); i++) {
		if (0 == strncmp(line + 1, valid_sections[i], (int)strlen(line) - 3)) {
			break;
		}
	}

	return i;
}

void validateSection(int section, struct section *sec, char *line) {
	int i;
	char *tag = strtok(line, "=");
	char *value = strtok(NULL, "\n");

	switch (section) {
		case 0:
			for (i = 0; i < ARRAY_LENGTH(valid_info_tags); i++) {
				if (0 == strcmp(tag, valid_info_tags[i])) {
					++sec->tag_count;
					strcpy(sec->tag_value[sec->tag_count - 1].tag, tag);
					strcpy(sec->tag_value[sec->tag_count - 1].value, value);
					break;
				}
			}
			break;
		case 1:
			for (i = 0; i < ARRAY_LENGTH(valid_event1_tags); i++) {
				if (0 == strcmp(tag, valid_event1_tags[i])) {
					sec->tag_count++;
					strcpy(sec->tag_value[sec->tag_count - 1].tag, tag);
					strcpy(sec->tag_value[sec->tag_count - 1].value, value);
					break;
				}
			}
			break;
		case 2:
			for (i = 0; i < ARRAY_LENGTH(valid_event2_tags); i++) {
				if (0 == strcmp(tag, valid_event2_tags[i])) {
					sec->tag_count++;
					strcpy(sec->tag_value[sec->tag_count - 1].tag, tag);
					strcpy(sec->tag_value[sec->tag_count - 1].value, value);
					break;
				}
			}
			break;
	}
}

int main(void) {
	FILE *fp;
	int file_size;
	size_t bytes_read;
	char *ini_file_text_buffer;
	char *ini_buffer_ptr = NULL;

	struct section info;
	struct section event1;
	struct section event2;

	info.tag_count = 0;
	event1.tag_count = 0;
	event2.tag_count = 0;

	info.tag_value = malloc(sizeof(struct tagValue) * ARRAY_LENGTH(valid_info_tags));
	event1.tag_value = malloc(sizeof(struct tagValue) * ARRAY_LENGTH(valid_event1_tags));
	event2.tag_value = malloc(sizeof(struct tagValue) * ARRAY_LENGTH(valid_event2_tags));

	/* Open ini file */
	fp = fopen(INI_FILE_PATH, "r");

	if (NULL == fp) {
		printf("Could not find .ini file, exiting program.\n");
		exit(1);
	}

	/* Get file size to malloc buffer */
	file_size = getFileSize(fp);

	/* Malloc buffer == file size */
	ini_file_text_buffer = malloc(file_size);

	/* Read file */
	bytes_read = fread(ini_file_text_buffer, sizeof(char), file_size, fp);
	if (bytes_read != file_size) {
		printf("Error reading file, file size doesn't match bytes read. Exiting.\n");
		exit(2);
	}

	// use strtok_r so we can use strtok to parse tags
	char *str = strtok_r(ini_file_text_buffer, "\n", &ini_buffer_ptr);
	int line_counter = 1;
	int section_flag; // 0 = information, 1 = event1, 2 = event2

	while (NULL != str) {
		switch (str[0]) {
			// Ignore these cases
			case CR:
			case COMMENT:
				break;

			case SECTION:
				section_flag = getSection(str);
				break;
			default:
				// Tag value
				switch (section_flag) {
				case 0:
					validateSection(section_flag, &info, str);
					break;
				case 1:
					validateSection(section_flag, &event1, str);
					break;
				case 2:
					validateSection(section_flag, &event2, str);
					break;
				}
				break;
		}
		str = strtok_r(NULL, "\n", &ini_buffer_ptr);
		++line_counter;
	}

	int i;
	printf("Information section parsed: %d tags correctly\n", info.tag_count);
	if (info.tag_count) {
		for (i = 0; i < info.tag_count; i++) {
			printf("Tag: %s, Value: %s", info.tag_value[i].tag, info.tag_value[i].value);
		}
	}

	printf("\nEvent1 section parsed: %d tags correctly\n", event1.tag_count);
	if (event1.tag_count) {
		for (i = 0; i < event1.tag_count; i++) {
			printf("Tag: %s, Value: %s", event1.tag_value[i].tag, event1.tag_value[i].value);
		}
	}

	printf("\nEvent2 section parsed: %d tags correctly\n", event2.tag_count);
	if (event2.tag_count) {
		for (i = 0; i < event2.tag_count; i++) {
			printf("Tag: %s, Value: %s", event2.tag_value[i].tag, event2.tag_value[i].value);
		}
	}

	/* Clean up */
	free(ini_file_text_buffer);
	free(info.tag_value);
	free(event1.tag_value);
	free(event2.tag_value);
	fclose(fp);

	return 0;
}
