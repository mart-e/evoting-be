IF line-counter > prt-lines - 2
THEN DO:
    PUT UNFORMATTED SKIP(1) "../..     (" setup.sys-type 
                                          setup.org-type 
                                          setup.areaname 
                                          STRING(setup.orginator,"999")")".
    PAGE.
END.