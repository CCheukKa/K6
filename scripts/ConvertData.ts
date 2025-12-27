import fs from "fs";
import path from "path";

const root = path.resolve(__dirname, "..");
const outputDataDir = path.join(root, "data");
if (!fs.existsSync(outputDataDir)) {
    fs.mkdirSync(outputDataDir, { recursive: true });
}
const K6WebRoot = path.resolve(root, "K6-web");
const strokeJsonPath = path.join(K6WebRoot, "public", "strokeData.json");
const suggestionsJsonPath = path.join(K6WebRoot, "public", "suggestionsData.json");
const strokeTxtPath = path.join(outputDataDir, "strokeData.txt");
const suggestionsTxtPath = path.join(outputDataDir, "suggestionsData.txt");
if (!fs.existsSync(strokeJsonPath) || !fs.existsSync(suggestionsJsonPath)) {
    console.warn("K6-web data files not found. Running `bun run compile`.");
    const { execSync } = require("child_process");
    execSync("bun install", { cwd: K6WebRoot, stdio: "inherit" });
    execSync("bun run compile", { cwd: K6WebRoot, stdio: "inherit" });
    if (!fs.existsSync(strokeJsonPath) || !fs.existsSync(suggestionsJsonPath)) {
        throw new Error("K6-web data files still not found after compilation.");
    }
}

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
    if (!Array.isArray(jsonData)) { return 0; }

    const lines: string[] = [];
    for (const entry of jsonData) {
        for (const seq of entry.strokeSequences) {
            const code = seq.trim();
            const character = entry.character.trim();
            if (code && character) {
                lines.push(`${code}\t${character}`);
            }
        }
    }

    fs.writeFileSync(strokeTxtPath, lines.join("\n"), "utf8");
    return lines.length;
}

function writeSuggestions(jsonData: SuggestionsJson | null): number {
    if (!jsonData || typeof jsonData !== "object") { return 0; }

    const lines: string[] = [];
    for (const [key, suggestions] of Object.entries(jsonData)) {
        const cleanKey = key.trim();
        const cleanSuggestions = suggestions
            .map(s => s.trim())
            .filter(s => s);
        if (cleanKey && cleanSuggestions.length > 0) {
            lines.push(`${cleanKey}\t${cleanSuggestions.join(" ")}`);
        }
    }

    fs.writeFileSync(suggestionsTxtPath, lines.join("\n"), "utf8");
    return lines.length;
}

function main(): void {
    console.log("Converting JSON to IME text format...");

    const strokeJson = readJson<StrokeJsonEntry[]>(strokeJsonPath);
    const strokeCount = writeDictionary(strokeJson);
    console.log(`Wrote ${strokeCount} dictionary entries to ${strokeTxtPath}`);

    const suggestionsJson = readJson<SuggestionsJson>(suggestionsJsonPath);
    const suggestionCount = writeSuggestions(suggestionsJson);
    console.log(`Wrote ${suggestionCount} suggestion entries to ${suggestionsTxtPath}`);
}
main();