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