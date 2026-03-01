# Playground Front

This is a React application that is built and run in a Docker container. This README contains instructions for local development and Docker image building.

## Requirements

Before starting, ensure you have the following tools installed:

- Node (Long Term Support version)
- [npm](https://www.npmjs.com/) (or [yarn](https://yarnpkg.com/))
- [Docker](https://www.docker.com/)

## Local Development

### Backend port
If the backend is running on a different port (this is the case if you are not using the Docker setup) then you need to modify `public/env.js`, for example:

```js
window.runEnv = {
   apiUrl: 'http://localhost:8000/',
};
```

### Installing Dependencies

Install the dependencies:

   ```bash
   npm install
   ```

   or if you're using `yarn`:

   ```bash
   yarn install
   ```

### Running the Application

To start the application in development mode:

   ```bash
   npm start
   ```

   The application will be available at: `http://localhost:3000`

   You don't need to restart it for most changes, the server supports live reloading of changed source files.

### Testing the Application

To run tests for the application:

   ```bash
   npm test
   ```

   or if you're using `yarn`:

   ```bash
   yarn test
   ```

### Building the Application

To build the production version of the app:

   ```bash
   npm run build
   ```

   The build will be located in the `build/` folder.

## Working with Docker

### Building the Docker Image

Build the Docker image:

   ```bash
   docker build -t playground-front .
   ```

   This will create an image named `playground-front` using the `Dockerfile` in the project root.

### Running the Docker Container

Run the container with port mapping and a specific name:

   ```bash
   docker run -d -p 3000:80 --name playground-front playground-front
   ```

   The application will be accessible at: `http://localhost:3000`.

### Stopping and Removing the Container

1. To stop the container:

   ```bash
   docker stop playground-front
   ```

2. To remove the container:

   ```bash
   docker rm playground-front
   ```

## Additional Information

- If you want to modify the code and see live changes, it’s recommended to use local development without Docker.
