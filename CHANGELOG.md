## General
- Updated cerver to 2.0b-51 in service Dockerfiles
- Added dedicated service local runtime definition
- Added redis connection main pocket sources
- Refactored pocket service HTTP configuration
- Handling redis dependency in development
- Added base dedicated service cache methods
- Added base methods to update user cache stats

## Models
- Refactored role action model implementation
- Updated role model with latest internal methods
- Refactored category model implementation
- Refactored places model internal query methods
- Updated transactions model implementation
- Removed pocket related values from user model

## Controllers
- Removed obsolete users controllers methods
- Refactored roles controller dedicated methods
- Added methods to check if category exists

## Routes
- Removed users login & register routes handlers
- Returning not found error in catch all handler