/*
 * Adapted from https://tools.ietf.org/html/rfc2812#section-2.3.1

2.3.1 Message format in Augmented BNF

   The protocol messages must be extracted from the contiguous stream of
   octets.  The current solution is to designate two characters, CR and
   LF, as message separators.  Empty messages are silently ignored,
   which permits use of the sequence CR-LF between messages without
   extra problems.

   The extracted message is parsed into the components <prefix>,
   <command> and list of parameters (<params>).

    The Augmented BNF representation for this is:

   NOTES:
      1) After extracting the parameter list, all parameters are equal
         whether matched by <middle> or <trailing>. <trailing> is just a
         syntactic trick to allow SPACE within the parameter.

   Most protocol messages specify additional semantics and syntax for
   the extracted parameter strings dictated by their position in the
   list.  For example, many server commands will assume that the first
   parameter after the command is the list of targets, which can be
   described with:



  NOTES:
      1) The <hostaddr> syntax is given here for the sole purpose of
         indicating the format to follow for IP addresses.  This
         reflects the fact that the only available implementations of
         this protocol uses TCP/IP as underlying network protocol but is
         not meant to prevent other protocols to be used.

      2) <hostname> has a maximum length of 63 characters.  This is a
         limitation of the protocol as internet hostnames (in
         particular) can be longer.  Such restriction is necessary
         because IRC messages are limited to 512 characters in length.
         Clients connecting from a host which name is longer than 63
         characters are registered using the host (numeric) address
         instead of the host name.

      3) Some parameters used in the following sections of this
         documents are not defined here as there is nothing specific
         about them besides the name that is used for convenience.
         These parameters follow the general syntax defined for
         <params>.
*/


//  special    =  %x5B-60 / %x7B-7D
//                   ; "[", "]", "\", "`", "_", "^", "{", "|", "}"
#define special "\\[\\]\\\\`_\\^\\{\\|\\}"

//  digit      =  %x30-39                 ; 0-9
#define digit "0-9"

//  hexdigit   =  digit / "A" / "B" / "C" / "D" / "E" / "F"
#define hexdigit digit "A-F"

//  letter     =  %x41-5A / %x61-7A       ; A-Z / a-z
#define letter "A-Za-z"

//  user       =  1*( %x01-09 / %x0B-0C / %x0E-1F / %x21-3F / %x41-FF )
//                  ; any octet except NUL, CR, LF, " " and "@"
#define user "[^ @]+"

//  nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
#define nickname "[" letter special "][-" letter digit special "]*"

//  ip4addr    =  1*3digit "." 1*3digit "." 1*3digit "." 1*3digit
#define ip4addr "[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"

//  ip6addr    =  1*hexdigit 7( ":" 1*hexdigit )
//  ip6addr    =/ "0:0:0:0:0:" ( "0" / "FFFF" ) ":" ip4addr
#define ip6addr "(0:0:0:0:0:(0|FFFF):" ip4addr "|[" hexdigit "]+(:[" hexdigit "]+){7})"

//  hostaddr   =  ip4addr / ip6addr
#define hostaddr "(" ip4addr "|" ip6addr ")"

//  shortname  =  ( letter / digit ) *( letter / digit / "-" )
//                *( letter / digit )
//                  ; as specified in RFC 1123 [HNAME]
#define shortname "[" letter digit "][-" letter digit "]*"

//  hostname   =  shortname *( "." shortname )
#define hostname shortname "(\\." shortname ")*"

//  servername =  hostname
#define servername hostname

//  host       =  hostname / hostaddr
#define host "(" hostname "|" hostaddr ")"

//    prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
#define prefix "((" servername ")|(" nickname ")(" "(!("user"))?" "@(" host "))?)"

//    command    =  1*letter / 3digit
#define command "([" letter "]+|[" digit "]{3})"

#define params "(( [^: ][^ ]*)*)?"

#define trailing " :(.+)?"

#define message "^(:" prefix " )?" command params trailing "$"

#ifdef TERMINAL
#define TO_PRINT TERMINAL
#else
#define TO_PRINT message
#endif

#ifdef JUST_PREPROCESS
TO_PRINT
#else
int putchar(int);
int puts(const char *);

int
main()
{
	const char msg[] = TO_PRINT;
	const char *p;

#ifdef ESCAPE
	putchar('"');
	for (p = msg; *p != '\0'; p++) {
		switch (*p) {
		case '\\':
			putchar('\\');
			putchar('\\');
			break;
		case '\"':
			putchar('\\');
			putchar('"');
			break;
		default:
			putchar(*p);
			break;
		}
	}
	putchar('"');
	putchar('\n');
#else /* ESCAPE */
	puts(msg);
#endif /* !ESCAPE */

	return 0;
}
#endif /* !JUST_PREPROCESS */
