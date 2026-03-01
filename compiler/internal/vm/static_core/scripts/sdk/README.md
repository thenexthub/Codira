## Panda SDK for OHOS arm64

### Build SDK:
```sh
./build_sdk.sh /path/to/panda/sdk/destination [OPTIONS]
```

Note: see script `test.sh` for more details

### Test SDK:
```sh
./test_sdk [OPTIONS]
```

### Install SDK from local tarball
```sh
npm install /path/to/panda/sdk/destination/panda-sdk-1.0.0-devel.tgz
```

### Publish SDK
To determine destination registry for SDK NPM package add following lines to `~/.npmrc`:
```
@panda:registry=${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/packages/npm/
${CI_API_V4_URL#https?}/projects/${CI_PROJECT_ID}/packages/npm/:_authToken=${CI_JOB_TOKEN}
strict-ssl=false
```
`CI_API_V4_URL`, `CI_PROJECT_ID` and `CI_JOB_TOKEN` should be changed manually or exported as environment variables

Publish npm package with `npm publish` command
