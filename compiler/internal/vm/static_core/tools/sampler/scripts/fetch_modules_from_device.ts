/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const commander = require('commander');
const program = new commander.Command();
const path = require('path');
const fs = require('fs');
const nodeCmd = require('node-cmd');

class ModulesFetcher {
  // Path to hdc, or path to hdc with hdc server ip.
  private hdc_: string | undefined;
  private pathChecksumMap_: Map<string, number>;

  private moduleFile_: string = '';
  private outputDir_: string = '';
  private appName_: string = '';

  constructor() {
    this.pathChecksumMap_ = new Map<string, number>();
  }

  private parseCliArgs(): boolean {
    program
      .description('Fetch libraries from device in order to use them to create valide flamegraph.');
    program
      .requiredOption('-m, --module_file <type>', 'Input file with modules paths.')
      .requiredOption('-o, --output_dir <type>', 'Directory in which libs will be fetched.')
      .requiredOption('-a, --app_name <type>', 'Name of the application that was profiled.');
  
    program.parse(process.argv);
    
    const args = program.opts();
    this.moduleFile_ = args.module_file;
    this.outputDir_ = args.output_dir;
    this.appName_ = args.app_name;

    if (!fs.existsSync(this.outputDir_)) {
      console.log('Directory ', this.outputDir_, 'doesnt exist.');
      return false;
    }

    return true;
  }

  private isEnvVarsSet(): boolean {
    let isEnvVarSet: boolean = true;
  
    if (!process.env.HDC_PATH && !process.env.HDC_IP) {
      console.log('Error in isEnvVarsSet: Please export HDC_PATH or HDC_IP(with port).');
      isEnvVarSet = false;
    }
  
    return isEnvVarSet;
  }
  
  // file line format: <checksum path>
  private parseModuleFile(): boolean {
    const fileContent = fs.readFileSync(this.moduleFile_, 'utf-8');
    fileContent.split(/\r?\n/).forEach((line) => {
      if (line) {
        let splitParts: Array<string> = line.split(' ');
        let checksum: number = Number(splitParts[0]);
        let path: string = this.mapSandboxDirToPhysicalDir(splitParts[1]);

        if (!this.pathChecksumMap_.has(path)) {
          this.pathChecksumMap_.set(path, checksum);
        }
      }
    });
  
    return !(this.pathChecksumMap_.size === 0);
  }
  
  // According to OHOS docs we need to do mapping in this way:
  // /data/storage/el1/bundle ---> /data/app/el1/bundle/public/<PACKAGENAME>
  private mapSandboxDirToPhysicalDir(path: string): string {
    return path.replace(/\/data\/storage\/el1\/bundle/g, `/data/app/el1/bundle/public/${this.appName_}`);
  }
  
  public DoFetching(): boolean {
    this.pathChecksumMap_.forEach((checksum: number, path: string) => {
      if (!this.areLibrariesIdentical(path, checksum)) {
        nodeCmd.runSync(`${this.hdc_} file recv ${path} ${this.outputDir_}`, (err, data, stderr) => console.log(data));
      }
    });

    return true;
  }

  private areLibrariesIdentical(devicePath: string, deviceChecksum: number): boolean {
    let hostFilepath : string = path.join(this.outputDir_, path.basename(devicePath));
    if (!fs.existsSync(hostFilepath)) {
      return false;
    }
    
    let hostChecksum: number = this.getHostChecksumFromPandaFile(hostFilepath);
    return deviceChecksum === hostChecksum;
  }

  private getHostChecksumFromPandaFile(hostFilepath: string): number {
    let magicSize: number = 8;
    let magicArraySize: number = 1 * magicSize;
    let checksumOffset: number = magicArraySize;

    let data = fs.readFileSync(hostFilepath);

    // read checksum (uint32_t) as a little-endian
    return data.readUInt32LE(checksumOffset);
  }

  private getHdc(): string | undefined {
    if (process.env.HDC_IP) {
      return 'hdc'.concat(' -s ', process.env.HDC_IP);
    } else if (process.env.HDC_PATH) {
      return process.env.HDC_PATH;
    }

    return undefined;
  }

  public fetchModulesFromDevice(): void {
    if (!this.isEnvVarsSet()) {
      console.log('Error: enviroment variables is not set');
      return;
    }

    this.hdc_ = this.getHdc();
    if (this.hdc_ === undefined) {
      console.log('Error: cant get hdc.');
      return;
    }

    if (!this.parseCliArgs()) {
      console.log('Error: error when parsing CLI args.');
      return;
    }

    if (!this.parseModuleFile()) {
      console.log('Error: can not parse module file.');
      return;
    }

    if (!this.DoFetching()) {
      console.log('Error: Can not fetch modules from device.');
      return;
    }
  }
}

const fetcher = new ModulesFetcher();
fetcher.fetchModulesFromDevice();
