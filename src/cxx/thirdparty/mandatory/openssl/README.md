## `openssl` is utility source code to `SCORE` that handles calls from `SCORE` to the `libssl-dev` library

Nothing needs to be compiled in here as this will happen during the compilation of `SCORE`.
Make sure though that `libssl-dev` has been installed on the system where you run `SCORE`.

`libssl-dev` and `openssl` is used in `SCORE` to generate SHA256 checksums for provenance tracking of data artifacts.

