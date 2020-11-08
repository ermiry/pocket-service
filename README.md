# Ermiry's Tiny Pocket API Service

### Development
```
sudo docker run \
  -it \
  --name pocket --rm \
  -p 5000:5000 \
  -v /home/ermiry/Documents/ermiry/website/tiny-pocket-api:/home/pocket \
  -e CURR_ENV=development \
  -e PORT=5000 \
  -e PRIV_KEY=/home/pocket/keys/key.key -e PUB_KEY=/home/pocket/keys/key.pub \
  -e MONGO_APP_NAME=api -e MONGO_DB=pocket \
  -e MONGO_URI=mongodb://api:password@192.168.100.39:27017/pocket \
  -e CERVER_RECEIVE_BUFFER_SIZE=4096 -e CERVER_TH_THREADS=4 \
  ermiry/tiny-pocket-api:development /bin/bash
```

```
sudo docker run \
  -it \
  --name pocket --rm \
  -p 5002 --net ermiry \
  -v /home/ermiry/Documents/ermiry/website/ermiry-website/tiny-pocket-api:/home/pocket \
  -v /home/ermiry/Documents/ermiry/website/ermiry-website/jwt:/home/pocket/keys \
  -e CURR_ENV=development \
  -e PORT=5002 \
  -e PRIV_KEY=/home/pocket/keys/key.key -e PUB_KEY=/home/pocket/keys/key.pub \
  -e MONGO_URI=mongodb://pocket:pocketpassword@192.168.100.39:27017/ermiry \
  -e CERVER_RECEIVE_BUFFER_SIZE=4096 -e CERVER_TH_THREADS=4 \
  ermiry/tiny-pocket-api:development /bin/bash
```

## Routes

### Main

#### GET /api/pocket
Pocket top level route

#### GET api/pocket/version
Returns pocket-api service current version

#### GET api/pocket/auth
Used to test if jwt keys work correctly

#### POST api/users/register
Used by users to create a new account

#### GET api/pocket/transactions
Get all the authenticated user's transactions

#### POST api/pocket/transactions
A user has requested to create a new transaction

#### GET api/pocket/transactions/:id
Returns information about an existing transaction that belongs to a user

#### DELETE api/pocket/transactions/:id
Deletes an existing user's transaction

### Users

#### GET /api/users
Users top level route

#### POST api/users/login
Uses the user's supplied creedentials to perform a login and generate a jwt token

#### POST api/users/register
Used by users to create a new account