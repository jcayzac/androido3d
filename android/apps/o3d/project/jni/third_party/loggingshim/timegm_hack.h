#ifndef TIMEGM_HACK
#define TIMEGM_HACK

#if defined(ANDROID)

inline time_t timegm (struct tm *t) {
  time_t tl, tb;
  struct tm *tg;
  
  tl = mktime (t);
  if (tl == -1) {
    t->tm_hour--;
    tl = mktime (t);
    if (tl == -1)
    return -1; /* can't deal with contents of T */
    tl += 3600;
  }
  tg = gmtime (&tl);
  tg->tm_isdst = 0;
  tb = mktime (tg);
  if (tb == -1) {
    tg->tm_hour--;
    tb = mktime (tg);
    if (tb == -1)
    return -1; /* can't deal with output from gmtime */
    tb += 3600;
  }
  return (tl - (tb - tl));
}

#endif  //ANDROID

#endif  //TIMEGM_HACK