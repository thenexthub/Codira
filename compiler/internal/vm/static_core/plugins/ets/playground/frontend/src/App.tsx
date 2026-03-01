/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

import React from 'react';
import './App.css';
import MosaicApp from './components/mosaic/MosaicApp';
import {ThemeProvider} from './components/theme/ThemeContext';
import Header from './components/header/Header';
import {Provider} from 'react-redux';
import {store} from './store';
import { Notifications } from './components/notif/notification';
import { useShareFromUrl } from './utils/useShareFromUrl';

function AppContent(): JSX.Element {
  useShareFromUrl();

  return (
      <>
          <Header />
          <Notifications />
          <MosaicApp />
      </>
  );
}

function App(): JSX.Element {
  return (
      <Provider store={store}>
          <ThemeProvider>
              <AppContent />
          </ThemeProvider>
      </Provider>
  );
}

export default App;
