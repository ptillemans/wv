#include <stdlib.h>
#include <stdio.h>
/* Don't use GNOMEVFS unless it is requested via config.h
 */
#ifdef HAVE_GNOMEVFS
#undef HAVE_GNOMEVFS
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "wv.h"
#include "utf.h"

#ifdef HAVE_GNOMEVFS
#include <libgnomevfs/gnome-vfs.h>
#include "ms-ole-gnomevfs.h"
#endif

int
wvInit (void)
{
#ifdef HAVE_GNOMEVFS
  if (gnome_vfs_init ())
    {
      ms_ole_init (ms_ole_get_gnomevfs_fs());
      return 1;
    }
  return 0;
#else
  ms_ole_init (NULL);
  return 1;
#endif
}

int
wvInitParser (wvParseStruct * ps, char *path)
{
    int ret = 0, reason = 0;

#ifdef __GNUC__
    /* i heard that GNU C has something like __sinit() 
     * to reset all static variables
     */
#endif

    memset ( ps, 0, sizeof ( ps ) ) ;

    ps->userData = NULL;
    ps->lst = NULL;
    ps->intable = 0;
    ps->endcell = 0;
    ps->vmerges = NULL;
    ps->norows = 0;
    ps->cellbounds = NULL;
    ps->nocellbounds = 0;
    ps->fieldstate = 0;
    ps->fieldmiddle = 0;

    ps->charhandler = 0;
    ps->scharhandler = 0;
    ps->elehandler = 0;
    ps->dochandler = 0;

    ps->password[0] = 0;
    /* set up the token table tree for faster lookups */
    tokenTreeInit ();

    ret = wvOLEDecode (path, &ps->mainfd, &ps->tablefd0, &ps->tablefd1,
		       &ps->data, &ps->summary);

    switch (ret)
      {
      case 0:
	  break;
      case 2:
	  ret = wvOpenPreOLE (path, &ps->mainfd, &ps->tablefd0, &ps->tablefd1,
			      &ps->data, &ps->summary);
	  if (ret)
	      return (ret);
	  break;
      case 3:
	  wvError (("Bad Ole\n"));
	  return (3);
	  break;
      default:
	  return (-1);
	  break;
      }

    if (ps->mainfd == NULL)
      {
	  ret = 4;
	  wvOLEFree ();
	  wvError (("Not a word document\n"));
	  return (-1);
      }

    wvGetFIB (&ps->fib, ps->mainfd);

    ps->tablefd = wvWhichTableStream (&ps->fib, ps);

    ret = wvQuerySupported (&ps->fib, &reason);

    if ((ret & 0x7fff) != WORD8)
	ps->data = ps->mainfd;

    if ((ret != WORD8) && (ret != WORD7) && (ret != WORD6) && (ret != WORD2))
	/* WORD2 test */
      {
	  /* return the errors and the encrypted files */
	  if (!(ret & 0x8000))
	      wvError (("%s\n", wvReason (reason)));
	  return (ret);
      }
    ret = 0;
    return ret;
}

void
wvSetPassword (const char *pass, wvParseStruct * ps)
{
    int i = 0, len;
    
    char * password = (char *)pass; /* causes no harm */

    /* at this stage we are passing in an utf-8 password and
       later converting it to unicode, we should use the generic
       available mb to wide char stuff, but that isnt very prevalent
       yet, and this is the only time i think i go from utf to 
       unicode */

    while (*password)
      {
	  len = our_mbtowc (&(ps->password[i]), password, 5);
	  i++;
	  password += len;
	  if (i == 16)
	      break;
      }
    ps->password[i] = 0;
}

int
wvOpenPreOLE (char *path, wvStream ** mainfd, wvStream ** tablefd0,
	      wvStream ** tablefd1, wvStream ** data, wvStream ** summary)
{
    int ret = -1;
    U16 magic;
    FILE *input;

    input = fopen (path, "rb");
    if (input == NULL)
      {
	  wvError (("Cannot open file $s\n", path));
	  return (-1);
      }

    wvStream_FILE_create (mainfd, input);

    *tablefd0 = *mainfd;
    *tablefd1 = *mainfd;
    *data     = *mainfd;
    *summary  = *mainfd;

    magic = read_16ubit (*mainfd);
    if (0xa5db == magic)
      {
	  wvError (
		   ("Theres a good chance that this is a word 2 doc of nFib %d\n",
		    read_16ubit (*mainfd)));
	  wvStream_rewind (*mainfd);
	  /* return(-1); */
	  return (0);
      }
    else if (0x37fe == magic)
      {
	  wvError (
		   ("Theres a good chance that this is a word 5 doc of nFib %d\n",
		    read_16ubit (*mainfd)));
	  wvStream_rewind (*mainfd);
	  return (0);
      }
    return (ret);
}
