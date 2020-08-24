# CHECKLIST: ADD NEW THIRD-PARTY LIBRARY

1. Create library folder under `thirdparty`

2. Prepare third-party library sources:
   - Download latest release of the thirdparty library.
   - Unpack it.
   - Rename library folder to be library_name-library_version.
   - `tar cvf library_name-library_version.tar library_name-library_version`
   - `xz -v9 library_name-library_version.tar`
   - Add to source control resulting `.tar.xz` archive.
   - DO NOT USE gzip or bzip2, use only xz with maximum compression level (-9)

3. Extract license file from the library source, put it into `thirdparty/libraryname`
   under name `LICENSE-library_version`. Add to source control.

4. Edit file `thirdparty/thirdparty_versions.sh`, add new variables, keep alpabetic order:
   - `export SIODB_LIBRARYNAME_VERSION=library version`
   - `export SIODB_LIBRARYNAME_PREFIX=${SIODB_TP_ROOT}/protobuf-${SIODB_LIBRARYNAME_VERSION}`

5. Edit `docs/dev/Build.md`, add library build instrutions.

6. Edit `mk/ThirdPartyLibVersions.mk`, add there new variable
   `LIBRARYNAME_VERSION=library_version`, keep in the alpabetic order.

7. Edit `mk/ThirdPartyLibs.mk` add there lines like these
   (keep libraries in the alphabetic order):

   ```makefile
   # Library_Name
   LIBRARYNAME_ROOT:=$(THIRD_PARTY_ROOT)/library_name-$(LIBRARYNAME_VERSION)
   CXX_INCLUDE+=-isystem $(LIBRARYNAME_ROOT)/include/library_name-$(LIBRARYNAME_VERSION)
   LDFLAGS+=-L$(LIBRARYNAME_ROOT)/$(OS_LIBDIR)/library_name-$(LIBRARYNAME_VERSION) \
      -Wl,-rpath -Wl,$(LIBRARYNAME_ROOT)/$(OS_LIBDIR)/library_name-$(LIBRARYNAME_VERSION)
   ```

8. Edit file `NOTICE` add line like this (keep in the alpabetic order):
   - `- Library_Name: thirdparty/library_name/LICENSE-library_version.txt`
