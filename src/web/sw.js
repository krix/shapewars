// Shapewars service worker: install the game once, then start offline.
const CACHE = 'shapewars-v1';
const CORE = [
    './',
    './index.html',
    './shapewars.js',
    './shapewars.wasm',
    './manifest.webmanifest',
    './icon-180.png',
    './icon-192.png',
    './icon-512.png',
    './icon-maskable-512.png'
];

self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(CACHE)
            .then((cache) => cache.addAll(CORE))
            .then(() => self.skipWaiting())
    );
});

self.addEventListener('activate', (event) => {
    event.waitUntil(
        caches.keys()
            .then((keys) => Promise.all(
                keys.filter((key) => key !== CACHE).map((key) => caches.delete(key))
            ))
            .then(() => self.clients.claim())
    );
});

self.addEventListener('fetch', (event) => {
    const request = event.request;
    if (request.method !== 'GET') return;

    const url = new URL(request.url);
    if (url.origin !== self.location.origin) return;

    event.respondWith((async () => {
        const cache = await caches.open(CACHE);

        try {
            // Network first, so a rebuilt game is picked up immediately.
            const response = await fetch(request);
            if (response && response.ok) {
                cache.put(request, response.clone());
            }
            return response;
        } catch (error) {
            const cached = await cache.match(request, { ignoreSearch: true });
            if (cached) return cached;
            if (request.mode === 'navigate') {
                return cache.match('./index.html');
            }
            throw error;
        }
    })());
});
