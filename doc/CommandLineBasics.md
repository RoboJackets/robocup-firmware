## Navigation
To navigate what directory we are in we use cd.

Go to directory: `cd sample_dir_name`

Go to parent directory: `cd ..`

Go to home directory: `cd ~`

Go back to previous directory: `cd -`

List directory contents: `ls`

List all directory contents along with metadata: `ls -l` or `ll`

List all directory contents including hidden files: `ls -A`

List directory (wildcard matching): `ls *.txt`

List all files of type: `find . -name "*.txt" -print`

## Manipulation

Make (empty) directory: `mkdir sample-dirname`

Remove directory with all contents (-r recursive): `rm -r sample-dirname/`

Remove directory with all contents but force (-f): `rm -rf sample-dirname/`

Make (empty) file: `touch sample-filename.txt`

Duplicate file: `cp source-filename.txt destination-filename.txt`

Move/Rename file: `mv source-filename.txt destination-filename.txt`

## Executing a file
To execute a file type the name of the file: `./directory/executable-to-run`

In order to be executed the file must have executable permissions (see permissions).

## Permissions
To check permissions on a file use list directory with  `ls -l`

The 3 groupings of rwx indicate read write execute permissions for owner, group, and everyone in that order.
Ex. rwxr-xr-x corresponds to owner has read, write, execute and then group and everyone have read and and execute.

To change permissions use chmod.

To add execute permissions on a file: `chmod +x sample-filename`

Permissions can also be set on directories in the same way as above. Which changes the default permissions of files created in those directories.

To change the permissions of all files in a directory recursively: `chmod +x -R sample-dirname/`

Permissions set as above change the owner you can set permissions for the owner, group, and all others at the same time with with numerical permissions such as: `chmod 755 index.php`
Which corresponds to rwx permissions for the owner rx permissions for group and all others.
The number used can be determined by adding up the numbers below for a specific permissions set:
```
4 - access/read (r)
2 - modify/write (w)
1 - execute (x)
```
The 7 for owner is derived as 4 + 2 + 1

Change owner: `chown root index.php` (`root` is the username)

Change group: `chgrp group index.php` (`www-data` is the groupname)

Users can have multiple secondary groups but only 1 primary group you should usually only add users with secondary group.

To add a user to a group as their secondary group use: `usermod -a -G examplegroup exampleusername`

To add a user to a group as their primary group use: `usermod -a -g examplegroup exampleusername`
