import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  build: {
    // "npm run build" writes the finished site into ../web, which server.exe serves.
    outDir: '../web',
    emptyOutDir: true,
    // The app is ~610 kB (mostly the Recharts library). It is served from this computer,
    // not over the internet, so that size is fine; raise Vite's 500 kB warning limit.
    chunkSizeWarningLimit: 800,
  },
  server: {
    // "npm run dev": Vite serves the page on http://localhost:5173 and passes every
    // /api request on to server.exe, so the C++ program still does all the work.
    // (server.exe must be running.)
    proxy: {
      '/api': { target: 'http://localhost:8080', changeOrigin: true },
    },
  },
});
