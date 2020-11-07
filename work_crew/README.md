---
title: Workcrew 
---

Run the program with two arguments. 
A string and a file path.

The program will queue the file path to the work crew. A crew member will determine whether the file path is a file or a directory.
* if a file
	* search file for string
* if directory
	* use readdir_r to find all directories and regular files 
		* and queue each to worker
	* each file containing string returned

