#Generator and Consumer problem Simulation
[![Build Status](https://travis-ci.org/zeruniverse/generator_consumer_simulation.svg?branch=master)](https://travis-ci.org/zeruniverse/generator_consumer_simulation)    
  
#How to use  
1. Input 1 to run or 2 to set your number of tools & operators  
2. The program then generates threads to simulate this situation and the status will be output to the screen (refresh every 500ms)  
3. Use combination key ctrl+z to pause/resume the program and ctrl+c to terminate the program  
  
  
**IMPORTANT: THIS PROGRAM SHOULD RUN IN FULL-SCREEN COMMAND-LINE-WINDOW to display normally.**  
  
The functions dealing with Ctrl+Z and Ctrl+C to pause/resume/terminate program are written by myself.   
  
Platform: ArchLinux, gcc  
Nov 3, 2014  
  
LISENCE: GNU GPL 3.0
  
**You must give appropriate credit (link to this repo) if you use (part of) this project IN YOUR WORK. Appropriate copyright info is also required by GNU GPL 3.0**
  
#Problem Description  
There are 3 generators and each produces a unique kind of material independently. All these materials  are stored in a input buffer with size 10 before they are forwarded to the operators. We have 3 operators with same priority who are responsible for  producing the products based on these materials. Each product needs 2 different kinds of materials. Each time an operator needs 2 tools for this purpose. There are totally 3 tools provided for these operators. An operator can only process one product  at one time.  When an operator gets both the materials and tools, he can produce a product within a limited time varied from 0.01 second to 1 second. Otherwise, he has to wait until all the necessities are met.  He can grab the materials or tools first, it does not matter, but he can only get one thing at one time. If an operator decides to make another product before he starts to make the current product , he can put the materials and tools back and re-get the new materials and tools. But he has to put the tools back after he finishes a product because other operators may need these tools. All the products are put into a size-unlimited output queue.  An operator cannot start a new product before he puts the product into the output queue. Some restrictions may apply to these products: 1) No same products can be next to each other in this queue.  We say that two products are same if they are made from the same kinds of materials. 2) The difference of the number of any two kinds of products produced should be less than 10, for example, we can have 10 of product A and 15 of product B, but it is not allowed if we have 10 of A and 21 of B because the difference is 11 which is larger than 10. 