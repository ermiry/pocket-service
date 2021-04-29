## General
- Removed custom mongo driver sources
- Refactored environment methods
- Refactored Dockerfiles steps
- Added more compilation flags in makefile
- Updated action model methods & fields
- Updated Dockerfiles to use cerver version 2.0b-33
- Updated Dockerfiles to use cmongo version 1.0b-12

## Transactions
- Updated transaction model with new cmongo types
- Added dedicated transactions model methods
- Updated transactions controller to use cmongo types
- Added transaction_get_by_oid_and_user_to_json ()
- Added trans to json middle-ware in trans controller
- Updated transactions get handlers with new methods

## Categories
- Refactored category methods to use cmongo model
- Added dedicated crud methods in category model
- Added category_get_by_oid_and_user_to_json ()
- Added category to json middle-ware in categories controller
- Updated categories get handlers with new methods

## Places
- Refactored place model to use new methods
- Added dedicated crud methods in place model
- Added place_get_by_oid_and_user_to_json ()
- Added place to json middle-ware in places controller
- Updated places get handlers with new methods

## Users
- Moved user register & login logic to users controller
- Refactored user model definitions in sources

## Tests
- Added dedicated test header with definitions
- Added base curl methods to be used in tests
- Added base successful user login & register tests
- Added base transactions, categories & places integration tests
- Added tests compilation rules in makefile