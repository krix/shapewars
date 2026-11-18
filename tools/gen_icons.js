// Generates the PWA icons without any external image dependencies.
// Usage: node tools/gen_icons.js
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const OUT_DIR = path.join(__dirname, '..', 'src', 'web');

const CRC_TABLE = (() => {
    const table = new Uint32Array(256);
    for (let n = 0; n < 256; n++) {
        let c = n;
        for (let k = 0; k < 8; k++) {
            c = (c & 1) ? (0xedb88320 ^ (c >>> 1)) : (c >>> 1);
        }
        table[n] = c >>> 0;
    }
    return table;
})();

function crc32(buffer) {
    let c = 0xffffffff;
    for (let i = 0; i < buffer.length; i++) {
        c = CRC_TABLE[(c ^ buffer[i]) & 0xff] ^ (c >>> 8);
    }
    return (c ^ 0xffffffff) >>> 0;
}

function chunk(type, data) {
    const typeBuffer = Buffer.from(type, 'ascii');
    const body = Buffer.concat([typeBuffer, data]);
    const out = Buffer.alloc(12 + data.length);
    out.writeUInt32BE(data.length, 0);
    body.copy(out, 4);
    out.writeUInt32BE(crc32(body), 8 + data.length);
    return out;
}

function encodePng(width, height, rgba) {
    const signature = Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]);
    const ihdr = Buffer.alloc(13);
    ihdr.writeUInt32BE(width, 0);
    ihdr.writeUInt32BE(height, 4);
    ihdr[8] = 8;  // bit depth
    ihdr[9] = 6;  // RGBA
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;

    const stride = width * 4;
    const raw = Buffer.alloc((stride + 1) * height);
    for (let y = 0; y < height; y++) {
        raw[y * (stride + 1)] = 0; // no filter
        rgba.copy(raw, y * (stride + 1) + 1, y * stride, (y + 1) * stride);
    }

    return Buffer.concat([
        signature,
        chunk('IHDR', ihdr),
        chunk('IDAT', zlib.deflateSync(raw, { level: 9 })),
        chunk('IEND', Buffer.alloc(0))
    ]);
}

function clamp01(v) {
    return v < 0 ? 0 : (v > 1 ? 1 : v);
}

function mix(a, b, t) {
    return a + (b - a) * t;
}

function smoothstep(edge0, edge1, x) {
    const t = clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3 - 2 * t);
}

function distToSegment(px, py, ax, ay, bx, by) {
    const vx = bx - ax;
    const vy = by - ay;
    const wx = px - ax;
    const wy = py - ay;
    const len2 = vx * vx + vy * vy;
    let t = len2 > 0 ? (wx * vx + wy * vy) / len2 : 0;
    t = clamp01(t);
    const dx = px - (ax + vx * t);
    const dy = py - (ay + vy * t);
    return Math.sqrt(dx * dx + dy * dy);
}

function pointInTriangle(px, py, tri) {
    const [a, b, c] = tri;
    const d1 = (px - b[0]) * (a[1] - b[1]) - (a[0] - b[0]) * (py - b[1]);
    const d2 = (px - c[0]) * (b[1] - c[1]) - (b[0] - c[0]) * (py - c[1]);
    const d3 = (px - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (py - a[1]);
    const hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    const hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(hasNeg && hasPos);
}

function sampleIcon(x, y, contentScale) {
    // Background: deep space with a soft gold glow behind the shape.
    const r = Math.sqrt((x - 0.5) * (x - 0.5) + (y - 0.52) * (y - 0.52));
    const bgT = smoothstep(0.02, 0.72, r);
    let cr = mix(0x1b / 255, 0x05 / 255, bgT);
    let cg = mix(0x24 / 255, 0x06 / 255, bgT);
    let cb = mix(0x40 / 255, 0x0d / 255, bgT);

    const glow = Math.max(0, 1 - r / 0.5);
    cr += 0.11 * glow * glow;
    cg += 0.075 * glow * glow;
    cb += 0.02 * glow * glow;

    // Upward triangle, scaled around its visual centre.
    const cx = 0.5;
    const cy = 0.52;
    const baseTri = [
        [0.5, 0.16],
        [0.15, 0.80],
        [0.85, 0.80]
    ];
    const tri = baseTri.map(([px, py]) => [
        cx + (px - cx) * contentScale,
        cy + (py - cy) * contentScale
    ]);

    const inside = pointInTriangle(x, y, tri);
    const edgeDist = Math.min(
        distToSegment(x, y, tri[0][0], tri[0][1], tri[1][0], tri[1][1]),
        distToSegment(x, y, tri[1][0], tri[1][1], tri[2][0], tri[2][1]),
        distToSegment(x, y, tri[2][0], tri[2][1], tri[0][0], tri[0][1])
    );
    const outline = 0.042 * contentScale;

    if (inside) {
        const top = Math.min(tri[0][1], tri[1][1], tri[2][1]);
        const bottom = Math.max(tri[0][1], tri[1][1], tri[2][1]);
        const t = clamp01((y - top) / (bottom - top));
        // Gold gradient from pale yellow to orange.
        cr = mix(1.0, 1.0, t);
        cg = mix(0.92, 0.53, t);
        cb = mix(0.45, 0.03, t);
    }

    if (edgeDist < outline * 0.5) {
        // Bright white outline with a faint warm rim.
        cr = 0.98;
        cg = 0.985;
        cb = 1.0;
    } else if (inside && edgeDist < outline * 0.95) {
        const t = (edgeDist - outline * 0.5) / (outline * 0.45);
        cr = mix(0.98, 1.0, t);
        cg = mix(0.985, 0.92, t);
        cb = mix(1.0, 0.45, t);
    }

    return [cr, cg, cb];
}

function renderIcon(size, contentScale) {
    const out = Buffer.alloc(size * size * 4);
    const samples = 4;
    const sampleCount = samples * samples;

    for (let y = 0; y < size; y++) {
        for (let x = 0; x < size; x++) {
            let r = 0;
            let g = 0;
            let b = 0;
            for (let sy = 0; sy < samples; sy++) {
                for (let sx = 0; sx < samples; sx++) {
                    const px = (x + (sx + 0.5) / samples) / size;
                    const py = (y + (sy + 0.5) / samples) / size;
                    const c = sampleIcon(px, py, contentScale);
                    r += c[0];
                    g += c[1];
                    b += c[2];
                }
            }
            r /= sampleCount;
            g /= sampleCount;
            b /= sampleCount;

            const i = (y * size + x) * 4;
            out[i] = Math.round(clamp01(r) * 255);
            out[i + 1] = Math.round(clamp01(g) * 255);
            out[i + 2] = Math.round(clamp01(b) * 255);
            out[i + 3] = 255;
        }
    }
    return out;
}

function writeIcon(name, size, contentScale) {
    const pixels = renderIcon(size, contentScale);
    const png = encodePng(size, size, pixels);
    fs.writeFileSync(path.join(OUT_DIR, name), png);
    console.log(`${name}: ${size}x${size}, ${png.length} bytes`);
}

fs.mkdirSync(OUT_DIR, { recursive: true });
writeIcon('icon-180.png', 180, 0.82);
writeIcon('icon-192.png', 192, 0.82);
writeIcon('icon-512.png', 512, 0.82);
writeIcon('icon-maskable-512.png', 512, 0.58);
