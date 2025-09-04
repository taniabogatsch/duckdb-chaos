# DuckdbChaos

This repository is based on https://github.com/duckdb/extension-template, check it out if you want to build and ship your own DuckDB extension.

---

### 🚧 WORK IN PROGRESS 🚧

DuckDBChaos allow you to invoke exceptions and signals.

#### Exceptions

```sql
SELECT duckdb_chaos_exception('hello', 'CATALOG');
Catalog Error:
hello
```

```sql
SELECT duckdb_chaos_exception('hello', 'INTERNAL');
INTERNAL Error:
hello

Stack Trace:

0        duckdb::Exception::Exception(duckdb::ExceptionType, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&) + 64
1        duckdb::DuckDBChaosExceptionFun(duckdb::DataChunk&, duckdb::ExpressionState&, duckdb::Vector&)::'lambda'(duckdb::string_t, duckdb::string_t)::operator()(duckdb::string_t, duckdb::string_t) const + 376
...
```

```sql
SELECT duckdb_chaos_exception('hello', 'FATAL');
FATAL Error:
Failed: database has been invalidated because of a previous fatal error. The database must be restarted prior to being used again.
Original error: "hello"

Stack Trace:

0        duckdb::Exception::Exception(duckdb::ExceptionType, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&) + 64
1        duckdb::FatalException::FatalException(duckdb::ExceptionType, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&) + 12
...
```

#### Signals

```sql
SELECT duckdb_chaos_signal('SIGSEGV');
zsh: segmentation fault  build/release/duckdb
```

```sql
SELECT duckdb_chaos_signal('SIGABRT');
zsh: abort      build/release/duckdb
```

```sql
SELECT duckdb_chaos_signal('SIGBUS');
zsh: bus error  build/release/duckdb
```


## Building
### Managing dependencies
DuckDB extensions uses VCPKG for dependency management. Enabling VCPKG is very simple: follow the [installation instructions](https://vcpkg.io/en/getting-started) or just run the following:
```shell
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export VCPKG_TOOLCHAIN_PATH=`pwd`/vcpkg/scripts/buildsystems/vcpkg.cmake
```
Note: VCPKG is only required for extensions that want to rely on it for dependency management. If you want to develop an extension without dependencies, or want to do your own dependency management, just skip this step. Note that the example extension uses VCPKG to build with a dependency for instructive purposes, so when skipping this step the build may not work without removing the dependency.

### Build steps
Now to build the extension, run:
```sh
make
```
The main binaries that will be built are:
```sh
./build/release/duckdb
./build/release/test/unittest
./build/release/extension/duckdb_chaos/duckdb_chaos.duckdb_extension
```
- `duckdb` is the binary for the duckdb shell with the extension code automatically loaded.
- `unittest` is the test runner of duckdb. Again, the extension is already linked into the binary.
- `duckdb_chaos.duckdb_extension` is the loadable binary as it would be distributed.

## Running the extension
To run the extension code, simply start the shell with `./build/release/duckdb`.

## Running the tests
Different tests can be created for DuckDB extensions. The primary way of testing DuckDB extensions should be the SQL tests in `./test/sql`. These SQL tests can be run using:
```sh
make test
```

### Installing the deployed binaries
To install your extension binaries from S3, you will need to do two things. Firstly, DuckDB should be launched with the
`allow_unsigned_extensions` option set to true. How to set this will depend on the client you're using. Some examples:

CLI:
```shell
duckdb -unsigned
```

Python:
```python
con = duckdb.connect(':memory:', config={'allow_unsigned_extensions' : 'true'})
```

NodeJS:
```js
db = new duckdb.Database(':memory:', {"allow_unsigned_extensions": "true"});
```

Secondly, you will need to set the repository endpoint in DuckDB to the HTTP url of your bucket + version of the extension
you want to install. To do this run the following SQL query in DuckDB:
```sql
SET custom_extension_repository='bucket.s3.eu-west-1.amazonaws.com/<your_extension_name>/latest';
```
Note that the `/latest` path will allow you to install the latest extension version available for your current version of
DuckDB. To specify a specific version, you can pass the version instead.

After running these steps, you can install and load your extension using the regular INSTALL/LOAD commands in DuckDB:
```sql
INSTALL duckdb_chaos
LOAD duckdb_chaos
```
