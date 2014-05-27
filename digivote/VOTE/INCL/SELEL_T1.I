/* Filename            : SELEL_T1.I                                     */
/*
     Automatically select elections for totalisation of same
     organisation type
*/
FOR EACH election,
    FIRST organisation WHERE     election.s-id            = verkdat
                         AND     election.org-type        = orgtype
                         AND     organisation.org-type    = orgtype
                         AND (   organisation.heading3[1] = area
                              OR organisation.heading3[2] = area
                              OR organisation.heading3[3] = area):
    ASSIGN
    election.e-pr = TRUE
    election.MAC  = "REFRESH"
    wz-totlevel   = election.totlevel.
END.
FOR EACH election WHERE election.e-pr      = FALSE
                    AND election.mandatory = FALSE
                    AND election.totlevel  = wz-totlevel:
    ASSIGN
    election.e-pr = TRUE
    election.MAC  = "REFRESH".
END.
/* eof */