#include "octree.h"


// Read until a '\n' character is reached or the buffer is full
// Returns true for a successful read, else if the buffer will overflow, false.
bool read_line(FILE* const fp, unsigned char* const buffer, const int bufferLen)
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
unsigned int parse_header(FILE* const fp)
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
Point** read_points(FILE* const fp, unsigned int numPoints)
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
		}
	}

	free(fieldData);
	return points;
}

// Iterate over the points array and find the min and max values for each field
void get_min_max(Point** points, unsigned int numPoints, float* fieldMins, float* fieldMaxs)
{
	for (int fieldIdx = 0; fieldIdx < NUM_FIELDS; fieldIdx++)
	{
		fieldMins[fieldIdx] = points[0]->coords[fieldIdx];
		fieldMaxs[fieldIdx] = points[0]->coords[fieldIdx];
	}

	for (unsigned int ptIdx = 1; ptIdx < numPoints; ptIdx++)
	{
		for (int fieldIdx = 0; fieldIdx < NUM_FIELDS; fieldIdx++)
		{
			if (points[ptIdx]->coords[fieldIdx] > fieldMaxs[fieldIdx])
			{
				fieldMaxs[fieldIdx] = points[ptIdx]->coords[fieldIdx];
			}
			else if (points[ptIdx]->coords[fieldIdx] < fieldMins[fieldIdx])
			{
				fieldMins[fieldIdx] = points[ptIdx]->coords[fieldIdx];
			}
		}
	}
}
