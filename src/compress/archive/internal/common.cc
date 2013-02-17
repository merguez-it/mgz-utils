#include "compress/archive/internal/common.h"

short rotate(short x, int n) {
  return ~(~(x >> n) ^ ((x & ~(~0 << n))<<((sizeof (x)*8)-n)));
}

void zip_time_to_dos_time(short time, int *hh, int *mm, int *ss) {
  *hh = (time>>11)&31;
  *mm = (time>>5)&63;
  *ss = (time<<1)&31;    
}

short dos_time_to_zip_time(int hh, int mm, int ss) {
  return (hh<<11) + (mm<<5) + (ss>>1);
}

void zip_date_to_dos_time(short date, int *yy, int *mm, int *dd) {
  *yy = ((date>>9)&127) + 1980;
  *mm = ((date>>5)&15);
  *dd = date&31; 
}

short dos_date_to_zip_date(int yy, int mm, int dd) {
  return ((yy - 1980)<<9) + (mm<<5) + dd;
} 
