//=================================================================
//
//        bsearch.c
//
//        Testcase for C library bsearch()
//
//=================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     ctarpy, jlarmour
// Contributors:  
// Date:          2000-04-30
// Description:   Contains testcode for C library bsearch() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

static int
Compar( const void *int1, const void *int2 )
{
    if ( *(int*)int1 < *(int*)int2 )
        return -1;
    else if ( *(int*)int1 == *(int*)int2 )
        return 0;
    else
        return 1;
} // Compar()

int
main( int argc, char *argv[] )
{
    int key;
    int *result;
    int i_array[] = {1, 5, 8, 35, 84, 258, 1022, 1022, 5300, 7372, 9029};

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "bsearch() function");

    // Test 1
    key = 8;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 8),
                        "bsearch() something in the middle");

    // Test 2
    key = 5300;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 5300),
                        "bsearch() something else in the middle");

    // Test 3
    key = 1;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 1),
                        "bsearch() first element");

    // Test 4
    key = 9029;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 9029),
                        "bsearch() last element");

    // Test 5
    key = 1022;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 1022),
                        "bsearch() duplicate element");

    // Test 6
    key = 2;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( result==NULL, "bsearch() nonexistent element");

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "bsearch() function");

} // main()


// EOF bsearch.c
