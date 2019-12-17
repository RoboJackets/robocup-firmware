### For new members
It is common practice to surround optional arguments to a command in square braces []. Thus we will try to use the same formatting in our brief documentation. Despite this please use the options prescribed by us in the docs unless you understand what the command will do without that option.


## Who is this Document for?
This document is primarily geared towards both new and old members who are working between both mtrain-firmware and robocup-firmware and thus need to know how to manage the package created by building mtrain firmware as well as those who would like a deeper understanding of what conan is doing for us in terms of package management.
It is not intended as a full guide to Conan as it only walks through the basic steps of updating a package for and changing requires but I will leave links to relevant documentation and give commentary as I go.


## Conan
Conan is a decentralized c++ package manager. This means that much like git it can work it multiple remotes to obtain the dependencies it requires and it can upload built packages to these remotes. The remotes are merely just package storage and do not build the packages themselves.
The default location for your conan directory will be ~/.conan/ . Here conan stores all user configurations and packages (under the data directory). It is ill advised to modify any of these files by hand but it is sometimes helpful to force rebuild packages by deleting the contents of the data directory.

To learn more about Conan see the introduction page on the official docs:
https://docs.conan.io/en/latest/introduction.html

## Dependency Management
TODO talk more about how conan deals with dependencies eg requires statement

## Binary Management
Conan also is capable of managing binaries targeted at specific platforms within these binaries. The settings for building for a specific platform are described in a conan profile which are stored in ~/.conan/profiles/ . For robocup-firmware you will be using the will be using the armv7hf profile to build for the mtrain platform.


## Creating a local conan package:
For those who are working on both mtrain and robocup firmware you may want to have seperate testing version of the mtrain package to do so would make a package with a different reference (which makes the package into a different folder in your ~/.conan/data/ folder) and then change the package dependencies in robocup-firmware to match the new package reference.

The below is used to create a local version of the conan package:
```
conan create . yourname/testing -pr armv7hf
```

This command follows the format:
```
conan create path [reference] [-pf profile_name]
```

Where:
reference - the versioning of the package formatted as user/channel (ex collin/testing)

path - the relative or absolute path to the location of the conanfile.py to run.

profile_name - the name of a conan profile indicating what target platform to build for (see binary management)

NOTE: path points to the directory of the conanfile.py to use. For mtrain-firmware the conanfile is at the root of the project. But for robocup-firmware the conanfile is at robot/conanfile.py thus you will need to cd into robot before running the above.

To change robocup-firmware to now use our newly created package rather than the one it pulls from the robojackets remote we must modify the definition of requires in the file robocup-firmware/robot/conanfile.py as follows:

```
    requires = (('mTrain/[1.0.0]@robojackets/stable'),
                ('eigen/[3.3.7]@conan/stable'))
```
becomes
```
    requires = (('mTrain/[1.0.0]@collin/testing'),
                ('eigen/[3.3.7]@conan/stable'))
```

Now our robocup firmware project will use our testing version of the mtrain firmware pacakge when it builds.

conan create official documentation:
https://docs.conan.io/en/latest/reference/commands/creator/create.html


## Uploading Conan Packages:
The package we created above is only available to us. To allow others to use it we need to upload the built package to a remote. For stable releases to the mtrain package we would use the below:
```
conan create . robojackets/stable -pr armv7hf
conan upload mTrain/[1.0.0]@robojackets/stable -r robojackets --all
```

This command follows the format:
```
conan upload package_name/[version]@user/channel -r remote_name [--all]
```
Where:

package_name/[version]@user/channel - full reference to the package fields are fairly self-explainitory.

NOTE: fields such as package_name and version are set in the conanfile.py that was used to build the package

-r remote_name  -  the remote to push to

--all  -  indicates that conan should push binaries as well as the sources to the remote

NOTE: The remote is what dictates where to push to. The @robojackets/stable is just the reference to the package of the form user/channel.
Conan references are generally formatted with the creators name as the user (or organization that is publishing the package in this case) followed by a tag as the channel (eg stable, staging, experimental, etc).

To push our test package to the robojackets remote we would use the following:
```
conan create . yourname/testing -pr armv7hf
conan upload mTrain/[1.0.0]@yourname/testing -r robojackets --all
```

conan upload official documentation:
https://docs.conan.io/en/latest/reference/commands/creator/upload.html

TODO talk more about how the conanfile itself works
