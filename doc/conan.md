## Creating a local conan package:
conan create . collin/testing -pr armv7hf

## Uploading Conan Package:
conan create . robojackets/stable -pr armv7hf
conan upload mTrain/[1.0.0]@robojackets/stable -r robojackets --all

Where:
package_name/[version]@<user/channel>
-r robojackets  -  indicates what remote to push to
--all  -  indicates that conan should push binaries as well as the sources to the remote
