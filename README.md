# Ermiry's Tiny Pocket API Service

### Development
```
sudo docker run \
  -it \
  --name pocket --rm \
  -p 5004:5004 --net ermiry \
  -v /home/ermiry/Documents/ermiry/ermiry-web/pocket-service:/home/pocket \
  -v /home/ermiry/Documents/ermiry/ermiry-web/keys:/home/keys \
  -e RUNTIME=development \
  -e PORT=5004 \
  -e CERVER_RECEIVE_BUFFER_SIZE=4096 -e CERVER_TH_THREADS=4 \
  -e CERVER_CONNECTION_QUEUE=4 \
  -e MONGO_APP_NAME=pocket -e MONGO_DB=ermiry \
  -e MONGO_URI=mongodb://pocket:password@mongo:27017/ermiry \
  -e PUB_KEY=/home/keys/key.pub \
  -e CONNECT_TO_REDIS=true -e REDIS_HOSTNAME=redis \
  ermiry/pocket-service:development /bin/bash
```

## Routes

### Main

#### GET /api/pocket
**Access:** Public \
**Description:** Pocket top level route \
**Returns:**
  - 200 on success

#### GET /api/pocket/version
**Access:** Public \
**Description:** Returns pocket service current version \
**Returns:**
  - 200 and version's json on success

#### GET /api/pocket/auth
**Access:** Private \
**Description:** Used to test if JWT keys work correctly \
**Returns:**
  - 200 on success
  - 401 on failed auth

### Transactions

#### GET /api/pocket/transactions
**Access:** Private \
**Description:** Get all the authenticated user's transactions \
**Returns:**
  - 200 on success
  - 400 on bad request
  - 401 on failed auth
  - 500 on server error

#### POST /api/pocket/transactions
**Access:** Private \
**Description:** A user has requested to create a new transaction \
**Returns:**
  - 200 on success
  - 400 on bad request
  - 401 on failed auth
  - 500 on server error

#### GET /api/pocket/transactions/:id/info
**Access:** Private \
**Description:** Returns information about an existing transaction that belongs to a user \
**Returns:**
  - 200 and transaction's json on success
  - 401 on failed auth
  - 404 on transaction not found

#### PUT /api/pocket/transactions/:id/update
**Access:** Private \
**Description:** A user wants to update an existing transaction \
**Returns:**
  - 200 on success updating user's transaction
  - 400 on bad request due to missing values
  - 401 on failed auth
  - 500 on server error

#### DELETE /api/pocket/transactions/:id/remove
**Access:** Private \
**Description:** Deletes an existing user's transaction \
**Returns:**
  - 200 on success deleting user's transaction
  - 400 on bad request
  - 401 on failed auth
  - 500 on server error

### Categories

#### GET /api/pocket/categories
**Access:** Private \
**Description:** Get all the authenticated user's categories \
**Returns:**
  - 200 and categories json on success
  - 401 on failed auth

#### POST /api/pocket/categories
**Access:** Private \
**Description:** A user has requested to create a new category \
**Returns:**
  - 200 on success creating category
  - 400 on failed to create new category
  - 401 on failed auth
  - 500 on server error

#### GET /api/pocket/categories/:id/info
**Access:** Private \
**Description:** Returns information about an existing category that belongs to a user \
**Returns:**
  - 200 and category's json on success
  - 401 on failed auth
  - 404 on category not found

#### PUT /api/pocket/categories/:id/update
**Access:** Private \
**Description:** A user wants to update an existing category \
**Returns:**
  - 200 on success updating user's category
  - 400 bad request due to missing values
  - 401 on failed auth
  - 500 on server error

#### DELETE /api/pocket/categories/:id/remove
**Access:** Private \
**Description:** Deletes an existing user's category \
**Returns:**
  - 200 on success deleting user's category
  - 400 on bad request
  - 401 on failed auth
  - 500 on server error

### Places

#### GET /api/pocket/places
**Access:** Private \
**Description:** Get all the authenticated user's places \
**Returns:**
  - 200 and places json on success
  - 401 on failed auth

#### POST /api/pocket/places
**Access:** Private \
**Description:** A user has requested to create a new place \
**Returns:**
  - 200 on success creating place
  - 400 on failed to create new place
  - 401 on failed auth
  - 500 on server error

#### GET /api/pocket/places/:id/info
**Access:** Private \
**Description:** Returns information about an existing place that belongs to a user \
**Returns:**
  - 200 and place's json on success
  - 401 on failed auth
  - 404 on place not found

#### PUT /api/pocket/places/:id/update
**Access:** Private \
**Description:** A user wants to update an existing place \
**Returns:**
  - 200 on success updating user's place
  - 400 bad request due to missing values
  - 401 on failed auth
  - 500 on server error

#### DELETE /api/pocket/places/:id/remove
**Access:** Private \
**Description:** Deletes an existing user's place \
**Returns:**
  - 200 on success deleting user's place
  - 400 on bad request
  - 401 on failed auth
  - 500 on server error
