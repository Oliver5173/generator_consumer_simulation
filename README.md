# Generator and Consumer problem Simulation
[![Build Status](https://travis-ci.org/zeruniverse/generator_consumer_simulation.svg?branch=master)](https://travis-ci.org/zeruniverse/generator_consumer_simulation)    

## Problem Description  
+ 3 generators each producing a unique kind of material independently.
+ All materials are stored in a input buffer of size 10 before being forwarded to operators.
+ 3 operators with same priority use materials to produce products.
+ Each product needs 2 different kinds of materials and 2 tools.
+ Totally 3 tools are provided.
+ An operator can only process one product at one time.
+ When an operator gets both the materials and tools, he can produce a product within a limited time varied from 0.01 second to 1 second.
+ An operator can grab the materials or tools first, it does not matter, but he can only get one thing at one time.
+ If an operator decides to make another product before he starts to make the current product, he can put the materials and tools back and re-get the new materials and tools.
+ He has to put tools back after finishing a product.
+ All the products are put into a size-unlimited output queue.
+ An operator cannot start a new product before he puts the product into the output queue.
+ Restrictions on output queue:
  + No same products can be next to each other. We say that two products are same if they are made from the same kinds of materials.
  + The difference of the number of any two kinds of products produced should be less than 10. We can have 10 of product A and 15 of product B, but it is not allowed if we have 10 of A and 21 of B because the difference is 11 which is larger than 10. 

## Build Instructions
1. Install pthread library
2. ``make``  
  
# How to use  
1. Input 1 to run or 2 to set your number of tools & operators  
2. The program generates threads to simulate this situation and the status will be output to the screen (refresh every 500ms)  
3. Use combination key ctrl+z to pause/resume the program and ctrl+c to terminate the program  
  
**IMPORTANT: THIS PROGRAM SHOULD RUN IN FULL-SCREEN COMMAND-LINE-WINDOW to display normally.**  
  
The functions dealing with Ctrl+Z and Ctrl+C to pause/resume/terminate program are written by myself.   
  
Platform: ArchLinux, gcc  
Nov 3, 2014  
  
LISENCE: GNU GPL 3.0
  
**You must give appropriate credit (link to this repo) if you use (part of) this project IN YOUR WORK. Appropriate copyright info is also required by GNU GPL 3.0**
