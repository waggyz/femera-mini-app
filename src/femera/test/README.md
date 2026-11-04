# Femera integration tests

This folder contains partial and full end-to-end integration tests.

- `femera::test::fems::Self`: orchestrates Femera integration self-tests
- `femera::test::fems::FE1e`: provides and tests single element geometric data and methods. These are also used for Phys and Algo tests.
- `femera::test::fems::Ptch`: provides and tests patch geometric data and methods. These are also used for Phys and Algo tests. Results can be compared to element single tiles provided by the Elem class.
- `femera::test::fems::Phys`: provides and tests material data and methods.
- `femera::test::fems::Algo`: provides and tests solver data and methods.


