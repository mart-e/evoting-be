/*@source sec=1*/
FIND FIRST setup   NO-LOCK.
FIND FIRST sysinfo NO-LOCK.

maskey = SUBSTR(STRING( sysinfo.offset),  2,2)
       + SUBSTR(ENTRY(1,setup.startup[1]),1,2)
       + SUBSTR(        sysinfo.serialNo, 5,2)
       + SUBSTR(ENTRY(1,setup.startup[1]),4,2)
       + SUBSTR(        sysinfo.userName, 8,2)
       + SUBSTR(ENTRY(1,setup.startup[1]),7,2)
       + SUBSTR(        sysinfo.licenseNo,1,2)
       + SUBSTR(ENTRY(1,setup.startup[1]),9,2).

/*@source sec=0*/          
