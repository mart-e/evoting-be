/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="ddcustom.h">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.3 $</revision>
  <archiveTimeStamp>$Date: 2005/09/12 12:27:04 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>ddcustom.h</fileName>
  <creationDate/>
  <rights/>
  <author/>
  <description>
    <fr>header file</fr>
    <nl>header file</nl>
  </description>
  <history/>
  <comment/>
</comments>
<header>*/

#define NDIG 20

#include "\vote\hlc\src\my_sha1.inc"
#define STRINGDIGEST SHA1_string
#define STREAMDIGEST SHA1_stream
#define MULTIDIGEST SHA1_multi

#include "\vote\hlc\src\digutils.inc"

/*</header>
</source>
<!-- eof -->*/
