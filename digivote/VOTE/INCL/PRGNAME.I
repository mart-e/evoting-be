CAPS(SUBSTR({1},1,(IF   R-INDEX({1},".") > 0 
                   THEN R-INDEX({1},".") - 1 
                   ELSE 8                   )))