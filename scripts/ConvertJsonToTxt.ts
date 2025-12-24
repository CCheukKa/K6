import fs from "fs";
import path from "path";

const root = path.resolve(__dirname, "..");
const strokeJsonPath = path.join(root, "data", "strokeData.json");
const suggestionsJsonPath = path.join(root, "data", "suggestionsData.json");
const strokeTxtPath = path.join(root, "data", "strokeData.txt");
const suggestionsTxtPath = path.join(root, "data", "suggestionsData.txt");

type StrokeJsonEntry = {
    character: string;
    strokeSequences: string[];
};

type SuggestionsJson = Record<string, string[]>;

function readJson<T>(filePath: string): T | null {
    if (!fs.existsSync(filePath)) {
        throw new Error(`Missing file: ${filePath}`);
    }
    const raw = fs.readFileSync(filePath, "utf8");
    if (!raw.trim()) return null;
    try {
        return JSON.parse(raw) as T;
    } catch (err) {
        if (err instanceof Error) {
            throw new Error(`Failed to parse JSON at ${filePath}: ${err.message}`);
        }
        throw err;
    }
}

function writeDictionary(jsonData: StrokeJsonEntry[] | null): number {
    if (!Array.isArray(jsonData)) return 0;

    // Preserve the original order from JSON: codes appear in the order of entries,
    // and characters per code appear in their first-seen order.
    const codeOrder: string[] = [];
    const map = new Map<string, { chars: string[]; seen: Set<string> }>();
    for (const entry of jsonData) {
        if (!entry || typeof entry.character !== "string" || !Array.isArray(entry.strokeSequences)) continue;
        const ch = entry.character.trim();
        if (!ch) continue;
        for (const seq of entry.strokeSequences) {
            if (typeof seq !== "string") continue;
            const code = seq.trim();
            if (!code) continue;
            if (!map.has(code)) {
                map.set(code, { chars: [], seen: new Set<string>() });
                codeOrder.push(code);
            }
            const bucket = map.get(code)!;
            if (!bucket.seen.has(ch)) {
                bucket.seen.add(ch);
                bucket.chars.push(ch);
            }
        }
    }

    if (map.size === 0) return 0;

    const header = [
        "# Chinese IME Dictionary",
        "# Format: code<TAB>character",
        "# Lines starting with # or ; are comments",
        "# File must be UTF-8 encoded",
        "",
    ];

    const lines: string[] = [];
    for (const code of codeOrder) {
        const bucket = map.get(code)!;
        for (const ch of bucket.chars) {
            lines.push(`${code}\t${ch}`);
        }
    }

    fs.writeFileSync(strokeTxtPath, header.concat(lines).join("\n"), "utf8");
    return lines.length;
}

function writeSuggestions(jsonData: SuggestionsJson | null): number {
    if (!jsonData || typeof jsonData !== "object") return 0;

    const lines: string[] = [];
    for (const ch of Object.keys(jsonData)) {
        const suggestions = jsonData[ch];
        const seen = new Set<string>();
        const list = Array.isArray(suggestions)
            ? suggestions.map(s => (typeof s === "string" ? s.trim() : "")).filter(Boolean)
            : [];
        for (const suggestion of list) {
            if (seen.has(suggestion)) continue;
            seen.add(suggestion);
            lines.push(`${ch}\t${suggestion}`);
        }
    }

    if (lines.length === 0) return 0;
    fs.writeFileSync(suggestionsTxtPath, lines.join("\n"), "utf8");
    return lines.length;
}

function main(): void {
    console.log("Converting JSON to IME text format...");

    const strokeJson = readJson<StrokeJsonEntry[]>(strokeJsonPath);
    const strokeCount = writeDictionary(strokeJson);
    if (strokeCount === 0) {
        console.warn("No stroke dictionary entries found; strokeData.txt not updated to avoid wiping data.");
    } else {
        console.log(`Wrote ${strokeCount} dictionary entries to ${strokeTxtPath}`);
    }

    const suggestionsJson = readJson<SuggestionsJson>(suggestionsJsonPath);
    const suggestionCount = writeSuggestions(suggestionsJson);
    if (suggestionCount === 0) {
        console.warn("No suggestions entries found; suggestionsData.txt not updated to avoid wiping data.");
    } else {
        console.log(`Wrote ${suggestionCount} suggestion entries to ${suggestionsTxtPath}`);
    }
}

try {
    main();
} catch (err) {
    if (err instanceof Error) {
        console.error(err.message);
    } else {
        console.error(err);
    }
    process.exit(1);
}
