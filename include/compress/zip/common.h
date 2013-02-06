#ifndef __ZIP_COMMON_H
#define __ZIP_COMMON_H

short rotate(short x, int n);
void zip_time_to_dos_time(short time, int *hh, int *mm, int *ss);
short dos_time_to_zip_time(int hh, int mm, int ss);
void zip_date_to_dos_time(short date, int *yy, int *mm, int *dd);
short dos_date_to_zip_date(int yy, int mm, int dd);

#endif // __ZIP_COMMON_H

