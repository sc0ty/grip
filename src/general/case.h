#ifndef __CASE_H__
#define __CASE_H__

#define IS_LOWER(x)		((x)>='a' && (x)<='z')
#define IS_UPPER(x)		((x)>='A' && (x)<='Z')
#define TO_LOWER(x)		(IS_UPPER(x) ? (x)+'a'-'A' : (x))
#define TO_UPPER(x)		(IS_LOWER(x) ? (x)-'a'+'A' : (x))
#define IS_ALPHA(x)		(IS_LOWER(x) || IS_UPPER(x))

#endif
