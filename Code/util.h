#ifndef CMMC_UTIL_H
#define CMMC_UTIL_H

#define __stringfy_1(x...) #x
#define __stringfy(x...) __stringfy_1(x)
#define __concat_1(x, y) x##y
#define __concat(x, y) __concat_1(x, y)

#define F_LIST(x) x,
#define F_STR_LIST(x) __stringfy(x),

#define VISITOR_FUNASGN(KIND) .VisitPtr_##KIND = Visit_##KIND,
#define VISITOR_DEF(ACCEPTOR, ID) \
  static ACCEPTOR##Visitor ID = (ACCEPTOR##Visitor) { \
    ACCEPTOR(VISITOR_FUNASGN) \
  };

#define F_DISPATCH(KIND) \
  case KIND: \
    visitor->VisitPtr_##KIND(p, arg); \
    break;

#endif  // CMMC_UTIL_H

