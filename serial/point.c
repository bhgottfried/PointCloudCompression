#include "octree.h"


static bool read_line(FILE* const fp, unsigned char* const buffer, const int bufferLen);
static unsigned int parse_header(FILE* const fp);
static Point** read_points(FILE* const fp, unsigned int numPoints);
static void get_min_max(PointSet* ptSet);


// Return a PointSet object after parsing .pcd file
PointSet* get_point_set(const char* const fileName)
{
	FILE* fp = fopen(fileName, "rb");
	if (!fp)
	{
		printf("Failed to open file.\n");
		return NULL;
	}

	unsigned int numPoints = parse_header(fp);
	if (!numPoints)
	{
		printf("Failed to parse .pcd header.\n");
		fclose(fp);
		return NULL;
	}
	
	Point** points = read_points(fp, numPoints);
	if (!points)
	{
		printf("Failed to parse point data.\n");
		fclose(fp);
		return NULL;
	}

	PointSet* ptSet = malloc(sizeof(*ptSet));
	ptSet->points = points;
	ptSet->numPoints = numPoints;
	get_min_max(ptSet);
	
	return ptSet;
}


// Free dynamically allocated memory in PointSet
void delete_point_set(PointSet* ptSet)
{
	for (unsigned int ptIdx = 0; ptIdx < ptSet->numPoints; ptIdx++)
	{
		free(ptSet->points[ptIdx]);
	}
	free(ptSet->points);
	free(ptSet);
}


// Write a pcd header with the number of points of the compressed octree
void write_header(FILE* const fp, unsigned int numPoints)
{
	const char version[] = "VERSION .7\n";
	const char fields[] = "FIELDS x y z\n";
	const char size[] = "SIZE 4 4 4\n";
	const char type[] = "TYPE f f f\n";
	const char count[] = "COUNT 1 1 1\n";
	char width[24] = "WIDTH ";		// Still need to convert numPoints into ASCII
	const char height[] = "HEIGHT 1\n";
	const char viewpoint[] = "VIEWPOINT 0 0 0 1 0 0 0\n";
	char points[24] = "POINTS ";	// Still need to convert numPoints into ASCII
	const char data[] = "DATA binary\n";

	unsigned int numDigits = 0;
	unsigned int numPointsCpy = numPoints;
	while (numPointsCpy)
	{
		numPointsCpy /= 10;
		numDigits++;
	}

	for (int digIdx = numDigits - 1; digIdx >= 0; digIdx--)
	{
		char currDigit = numPoints % 10 + '0';
		numPoints /= 10;
		width[digIdx + 6] = currDigit;
		points[digIdx + 7] = currDigit;
	}

	width[numDigits + 6] = '\n';
	width[numDigits + 7] = '\0';
	points[numDigits + 7] = '\n';
	points[numDigits + 8] = '\0';

	const char* adrs[] = { version, fields, size, type, count, width, height, viewpoint, points, data };
	for (int adrIdx = 0; adrIdx < 10; adrIdx++)
	{
		fwrite(adrs[adrIdx], sizeof(**adrs), strlen(adrs[adrIdx]), fp);
	}
}


// Read until a '\n' character is reached or the buffer is full
// Returns true for a successful read, else if the buffer will overflow, false.
static bool read_line(FILE* const fp, unsigned char* const buffer, const int bufferLen)
{
	for (int i = 0; i < bufferLen; i++)
	{
		buffer[i] = fgetc(fp);
		if (buffer[i] == '\n')
		{
			return true;
		}
	}
	return false;
}


// Header format:
// VERSION .7
// FIELDS x y z
// SIZE 4 4 4
// TYPE f f f
// COUNT 1 1 1
// WIDTH 10
// HEIGHT 1
// VIEWPOINT 0 0 0 1 0 0 0
// POINTS 10
// DATA binary
static unsigned int parse_header(FILE* const fp)
{
	// For now, ignore the header fields apart from the number of points
	// TODO add functionality for ascii encodings, different data types, and other fields to enable optimizations

	char buffer[50] = { 0 };
	unsigned int numPoints = 0;
	for (int i = 0; i < 10; i++)
	{
		if (!read_line(fp, buffer, 50))
		{
			return 0;	// Buffer overflow avoided
		}

		if (i == 8)		// If we're reading the number of points
		{
			for (int textIdx = 7; buffer[textIdx] != '\n'; textIdx++)
			{
				numPoints *= 10;
				numPoints += buffer[textIdx] - '0';
			}
		}
	}

	return numPoints;
}


// Create array of points by parsing input file.
static Point** read_points(FILE* const fp, unsigned int numPoints)
{
	float* fieldData = malloc(numPoints * NUM_FIELDS * FIELD_SIZE);
	if (fread(fieldData, FIELD_SIZE, numPoints * NUM_FIELDS, fp) != numPoints * NUM_FIELDS)
	{
		free(fieldData);
		return NULL;
	}

	Point** points = malloc(numPoints * sizeof(*points));
	for (unsigned int ptIdx = 0; ptIdx < numPoints; ptIdx++)
	{
		points[ptIdx] = malloc(sizeof(**points));
		int offset = ptIdx * NUM_FIELDS;
		for (int fieldIdx = 0; fieldIdx < NUM_FIELDS; fieldIdx++)
		{
			points[ptIdx]->coords[fieldIdx] = fieldData[offset + fieldIdx];
			points[ptIdx]->next = NULL;
		}
	}

	free(fieldData);
	return points;
}


// Iterate over the points array and find the min and max values for each field
static void get_min_max(PointSet* ptSet)
{
	for (int fieldIdx = 0; fieldIdx < NUM_FIELDS; fieldIdx++)
	{
		ptSet->mins[fieldIdx] = ptSet->points[0]->coords[fieldIdx];
		ptSet->maxs[fieldIdx] = ptSet->points[0]->coords[fieldIdx];
	}

	for (unsigned int ptIdx = 1; ptIdx < ptSet->numPoints; ptIdx++)
	{
		for (int fieldIdx = 0; fieldIdx < NUM_FIELDS; fieldIdx++)
		{
			if (ptSet->points[ptIdx]->coords[fieldIdx] > ptSet->maxs[fieldIdx])
			{
				ptSet->maxs[fieldIdx] = ptSet->points[ptIdx]->coords[fieldIdx];
			}
			else if (ptSet->points[ptIdx]->coords[fieldIdx] < ptSet->mins[fieldIdx])
			{
				ptSet->mins[fieldIdx] = ptSet->points[ptIdx]->coords[fieldIdx];
			}
		}
	}
}
