/*
 * Copyright (C) 2016 Orange - Franck Roudet
 *
 * This software is distributed under the terms and conditions of the 'Apache-2.0'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'http://www.apache.org/licenses/LICENSE-2.0'.
 */

/* Open IoT Kit for LPWAN
 *
 * Version:     0.1.0
 * Created:     2015-12-15 by Franck Roudet
 */
 
#ifndef __checkChangeMixin_H_
#define __checkChangeMixin_H_
 
class CheckChangeMixin {
   public:
   
  /**
   * check hardware changes
   */  
  virtual void checkChange(void)=0;


  /**
   * start hardware
   */  
  virtual void startHW(void) = 0;
  
 };
#endif
 
