# Deployment Guide

This guide explains how to deploy the Sudoku Engine frontend to GitHub Pages.

## Prerequisites

- GitHub repository with GitHub Actions enabled
- GitHub Pages enabled in repository settings

## Quick Start

1. **Enable GitHub Pages**:
   - Go to your repository on GitHub
   - Navigate to Settings → Pages
   - Under "Source", select **"GitHub Actions"**
   - Save

2. **Push to main branch**:
   ```bash
   git push origin main
   ```

3. **Wait for deployment**:
   - Go to Actions tab to see the build progress
   - Once complete, your site will be live at:
     `https://[your-username].github.io/sudoku-engine/`

## How It Works

The GitHub Actions workflow (`.github/workflows/deploy.yml`) does the following:

1. **Sets up environment**:
   - Installs Node.js 22
   - Sets up Emscripten SDK for WASM compilation

2. **Builds WASM module**:
   ```bash
   mkdir build-wasm && cd build-wasm
   emcmake cmake ..
   cmake --build .
   ```
   - This compiles the C++ code to WebAssembly
   - Outputs: `web/public/wasm/sudoku-engine.js` and `.wasm`

3. **Builds frontend**:
   ```bash
   cd web
   npm ci
   npm run build
   ```
   - Builds the Vue.js application
   - Outputs to: `web/dist/`

4. **Deploys to GitHub Pages**:
   - Uploads `web/dist/` as the Pages artifact
   - GitHub automatically serves it

## Manual Deployment

If you prefer to deploy manually or to a different hosting service:

### Step 1: Build WASM

```bash
# Install Emscripten SDK first (if not already installed)
# See: https://emscripten.org/docs/getting_started/downloads.html

mkdir -p build-wasm
cd build-wasm
emcmake cmake ..
cmake --build .
```

This will create:
- `web/public/wasm/sudoku-engine.js`
- `web/public/wasm/sudoku-engine.wasm`

### Step 2: Build Frontend

```bash
cd web
npm install
npm run build
```

This creates the production build in `web/dist/`.

### Step 3: Deploy

Upload the contents of `web/dist/` to your hosting service:
- **GitHub Pages**: Use the `gh-pages` branch or GitHub Actions
- **Netlify**: Drag and drop the `dist` folder
- **Vercel**: Connect your repo or deploy the `dist` folder
- **Any static host**: Upload `dist/` contents

## Troubleshooting

### WASM files not found

If the frontend can't find WASM files:
1. Ensure WASM build completed successfully
2. Check that files exist in `web/public/wasm/`
3. Verify the build copied files (CMakeLists.txt POST_BUILD step)

### Build fails in GitHub Actions

- Check Actions logs for specific errors
- Ensure Emscripten setup completed
- Verify Node.js version matches `package.json` engines

### Site loads but solver doesn't work

- Open browser console (F12) to check for errors
- Verify WASM files are loading (Network tab)
- Check CORS settings if deploying to custom domain

## Custom Domain

If using a custom domain:
1. Set `VITE_BASE_PATH` environment variable to `/` in your build
2. Or update `vite.config.ts` base path
3. Configure DNS as per GitHub Pages documentation

## Local Testing

Test the production build locally:

```bash
# Build WASM
mkdir -p build-wasm && cd build-wasm
emcmake cmake .. && cmake --build .

# Build frontend
cd ../web
npm run build

# Preview
npm run preview
```

This serves the production build at `http://localhost:4173`.

