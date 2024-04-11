#include "seal/seal.h"
#include "seal/symbolic.h"

int
main ()
{
  int ret;
  SealSymbol *a, *b;
  SealSymEntry *head;

  a = DeclareSymbol ("a");
  b = DeclareSymbol ("b");

  head = InitSymEntry ();
  ret = AddSym (head, a);
  printf ("%d\n", ret);
  ret = AddSym (head, a);
  printf ("%d\n", ret);
  ret = AddSym (head, b);
  printf ("%d\n", ret);

  DelSymEntry (head);

  UndeclareSymbol (a);
  UndeclareSymbol (b);

  return 0;
}
