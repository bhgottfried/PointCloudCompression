#include "single.h"

// Read in all of the raw text, return a string with the raw text from the input file, and save the number of characters
char* read_file(const char* const filename, unsigned int* const length)
{
	if (!filename)
	{
		printf("Filename must not be null.\n");
		return NULL;
	}

	FILE* fp = fopen(filename, "rb");
	if (!fp)
	{
		printf("%s could not be opened.\n", filename);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	*length = ftell(fp);		// Count number of characters in the file
	fseek(fp, 0, SEEK_SET);
	if (!(*length))
	{
		printf("File must not be empty.\n");
		fclose(fp);
		return NULL;
	}

	char* text = malloc(*length * sizeof(*text));
	if (!text)
	{
		printf("Memory allocation failed for file text.\n");
		fclose(fp);
		return NULL;
	}

	if (fread(text, sizeof(char), *length, fp) != *length)
	{
		printf("%s could not be read.\n", filename);
		fclose(fp);
		free(text);
		return NULL;
	}

	fclose(fp);
	return text;
}
