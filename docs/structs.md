# Structs

### TODO:
   - Implement more robust parsing wrt to structs

     Rn, if expressions get confused with something like this:

         fun takingBool(b: Bool) String {              
            if b { \" True !\" }                       
            else { \" False\" }                        
         }                                             

     b {...} is treated like struct cons
