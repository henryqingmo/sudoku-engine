#!/usr/bin/env python3

"""
killer_pack.py

Pack Killer Sudoku puzzles from a ZIP archive directly into per-cage binary files:
  cage-le-<N>.ks

Usage:
  # pack all cage folders 1..9 (default)
  python killer_pack.py puzzles.zip out_dir/

  # pack only cage size 5
  python killer_pack.py puzzles.zip out_dir/ --cage 5

  # inspect a packed file
  python killer_pack.py --inspect out_dir/cage-le-5.ks --count
  python killer_pack.py --inspect out_dir/cage-le-5.ks --show 17

  Data obtained from:
    https://github.com/UvA-KR16/KilerSudoku/blob/master/data/KRProject_1.zip
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
import zipfile
from pathlib import Path, PurePosixPath
from typing import Iterator, List, Optional, Tuple

MAGIC = b"KSF1"  # 4 bytes
VERSION = 1
HEADER_FMT = "<4sB3xI"  # magic(4), version(1), 3padding, count(uint32)
HEADER_SIZE = struct.calcsize(HEADER_FMT)  # 12 bytes
INDEX_ENTRY_FMT = "<Q"  # uint64 offset (absolute)
INDEX_ENTRY_SIZE = struct.calcsize(INDEX_ENTRY_FMT)  # 8

UINT32_LE = "<I"
UINT8 = "<B"

_coord_re = re.compile(r"\(\s*(\d+)\s*,\s*(\d+)\s*\)")
_ans_row_re = re.compile(r"\[([0-9,\s]+)\]")


# -------------------------
# Parsers for source files
# -------------------------
def parse_killer_cages(text: str) -> List[Tuple[int, List[Tuple[int, int]]]]:
    """
    Parse the cage lines from a .killer file.

    Returns list of (sum, [(r,c), ...]) for each cage line.
    We ignore any leading '1+' tokens or similar and extract only the coordinates inside parentheses.
    """
    cages = []
    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not line:
            continue
        # Sum is before '='
        if "=" in line:
            sum_part, rest = line.split("=", 1)
            try:
                s = int(sum_part.strip())
            except ValueError:
                # malformed line; skip
                continue
        else:
            # fallback: try first number before '('
            m = re.match(r"^\s*(\d+)", line)
            if not m:
                continue
            s = int(m.group(1))

        coords = []
        for cm in _coord_re.finditer(line):
            r = int(cm.group(1))
            c = int(cm.group(2))
            coords.append((r, c))
        if coords:
            cages.append((s, coords))
    return cages


def parse_ans_solution(text: str) -> List[int]:
    """
    Parse the .ans file to a flat list of 81 integers row-major.
    The .ans format appears to have 9 lines like: [1, 2, 3, ..., 9]
    Returns list length 81.
    """
    values: List[int] = []
    for line in text.splitlines():
        m = _ans_row_re.search(line)
        if not m:
            continue
        row_text = m.group(1)
        row_vals = [int(x) for x in row_text.split(",") if x.strip()]
        values.extend(row_vals)
    if len(values) != 81:
        raise ValueError(f"Parsed solution length is {len(values)} (expected 81)")
    # validate 1..9
    for v in values:
        if not (1 <= v <= 9):
            raise ValueError(f"Solution contains invalid digit: {v}")
    return values


# -------------------------
# Packing utilities
# -------------------------
def coord_to_byte(r: int, c: int) -> int:
    if not (0 <= r <= 15 and 0 <= c <= 15):
        raise ValueError("coords must fit in 4 bits")
    return (r << 4) | c


def build_puzzle_payload(
    solution81: List[int], cages: List[Tuple[int, List[Tuple[int, int]]]]
) -> bytes:
    """
    Return payload bytes (everything after payload_len) for a single puzzle:
       solution: 81 bytes
       uint8 num_cages
       for each cage:
         uint8 sum
         uint8 size
         size * uint8 coords (row<<4 | col)
    """
    bparts = []
    # solution
    bparts.append(bytes([int(x) for x in solution81]))  # 81 bytes

    # cages
    if len(cages) > 255:
        raise ValueError("too many cages (>255) for one puzzle")
    bparts.append(struct.pack(UINT8, len(cages)))
    for s, coords in cages:
        if not (0 <= s <= 255):
            raise ValueError("cage sum out of uint8 range")
        if len(coords) > 255:
            raise ValueError("cage size too large")
        bparts.append(struct.pack(UINT8, s))
        bparts.append(struct.pack(UINT8, len(coords)))
        coord_bytes = bytes(coord_to_byte(r, c) for (r, c) in coords)
        bparts.append(coord_bytes)

    payload = b"".join(bparts)
    return payload


# -------------------------
# OOP: Packer + Reader
# -------------------------
class KSPacker:
    def __init__(self, zip_path: str, out_dir: str, verbose: bool = True):
        self.zip_path = Path(zip_path)
        self.out_dir = Path(out_dir)
        self.verbose = verbose

    def _iter_pairs(
        self, z: zipfile.ZipFile, prefix_folder: str
    ) -> Iterator[Tuple[str, str]]:
        """
        Iterate over pairs (.killer, .ans) inside a given folder prefix (e.g. "5/").
        Yields (killer_name, ans_name) internal zip paths.
        """
        prefix = prefix_folder.rstrip("/") + "/"
        # gather mapping base->suffix
        path_map = {}
        for info in z.infolist():
            if info.is_dir():
                continue
            if not info.filename.startswith(prefix):
                continue
            p = PurePosixPath(info.filename)
            suffix = p.suffix.lower()
            if suffix not in (".killer", ".ans"):
                continue
            base = str(p.with_suffix(""))
            path_map.setdefault(base, {})[suffix] = info.filename
        for base, d in path_map.items():
            if ".killer" in d and ".ans" in d:
                yield (d[".killer"], d[".ans"])

    def pack_cage_folder(self, cage_n: int) -> int:
        """
        Pack all puzzles in zip folder "<cage_n>/" into file "cage-le-<n>.ks" in out_dir.
        Returns number of puzzles written.
        """
        out_file = self.out_dir / f"cage-le-{cage_n}.ks"
        if self.verbose:
            print(f"[INFO] Packing cage folder '{cage_n}/' -> {out_file}")

        with zipfile.ZipFile(self.zip_path, "r") as z:
            pairs = list(self._iter_pairs(z, str(cage_n)))
            count = len(pairs)
            if count == 0:
                if self.verbose:
                    print(f"[WARN] No puzzles found for folder {cage_n}/")
                return 0

            # Prepare output file: write header + placeholder index
            out_file.parent.mkdir(parents=True, exist_ok=True)
            with open(out_file, "wb") as f:
                # header: magic + version + padding + count
                f.write(struct.pack(HEADER_FMT, MAGIC, VERSION, count))
                # reserve index area
                index_base = f.tell()
                f.seek(index_base + count * INDEX_ENTRY_SIZE)

                offsets = []
                written = 0
                for killer_path, ans_path in pairs:
                    try:
                        killer_text = z.read(killer_path).decode("utf-8")
                        ans_text = z.read(ans_path).decode("utf-8")
                    except Exception as e:
                        if self.verbose:
                            print(
                                f"[WARN] failed to read {killer_path} / {ans_path}: {e}"
                            )
                        continue

                    try:
                        cages = parse_killer_cages(killer_text)
                        solution = parse_ans_solution(ans_text)
                    except Exception as e:
                        if self.verbose:
                            print(f"[WARN] parse failed for {killer_path}: {e}")
                        continue

                    payload = build_puzzle_payload(solution, cages)
                    payload_len = len(payload)
                    offset = f.tell()
                    f.write(struct.pack(UINT32_LE, payload_len))
                    f.write(payload)

                    offsets.append(offset)
                    written += 1
                    if self.verbose and (written % 200 == 0):
                        print(f"[INFO] written {written} puzzles...")

                # After writing puzzles, go back and write index
                end_of_puzzles = f.tell()
                f.seek(index_base)
                for off in offsets:
                    f.write(struct.pack(INDEX_ENTRY_FMT, off))
                f.seek(end_of_puzzles)

            if self.verbose:
                print(f"[INFO] Done. Wrote {written}/{count} puzzles to {out_file}")
            return written

    def pack_all(self, cages: Optional[List[int]] = None) -> None:
        """
        Pack all specified cage folders. By default, 2..9.
        """
        if cages is None:
            cages = list(range(2, 10))
        total = 0
        for c in cages:
            try:
                n = self.pack_cage_folder(c)
                total += n
            except Exception as e:
                print(f"[ERROR] packing folder {c}: {e}", file=sys.stderr)
        if self.verbose:
            print(f"[INFO] Total puzzles written across files: {total}")


class KSReader:
    def __init__(self, ks_path: str):
        self.ks_path = Path(ks_path)
        self._f = open(self.ks_path, "rb")
        self._read_header()

    def _read_header(self):
        self._f.seek(0)
        hdr = self._f.read(HEADER_SIZE)
        magic, version, count = struct.unpack(HEADER_FMT, hdr)
        if magic != MAGIC:
            raise ValueError("Not a KS file (bad magic)")
        if version != VERSION:
            raise ValueError("Unsupported version")
        self.count = count
        # read index
        self.index_offsets = []
        for i in range(self.count):
            data = self._f.read(INDEX_ENTRY_SIZE)
            (off,) = struct.unpack(INDEX_ENTRY_FMT, data)
            self.index_offsets.append(off)

    def close(self):
        self._f.close()

    def puzzle_count(self) -> int:
        return self.count

    def read_puzzle_by_index(
        self, idx: int
    ) -> Tuple[List[int], List[Tuple[int, List[Tuple[int, int]]]]]:
        """
        Return (solution81, cages) for puzzle at index idx (0-based).
        """
        if not (0 <= idx < self.count):
            raise IndexError("puzzle index out of range")
        off = self.index_offsets[idx]
        self._f.seek(off)
        (payload_len,) = struct.unpack(UINT32_LE, self._f.read(4))
        payload = self._f.read(payload_len)
        # parse payload
        if len(payload) < 81 + 1:
            raise ValueError("payload too short")
        sol = list(payload[0:81])
        pos = 81
        num_cages = payload[pos]
        pos += 1
        cages = []
        for _ in range(num_cages):
            s = payload[pos]
            pos += 1
            size = payload[pos]
            pos += 1
            coords = []
            for _ in range(size):
                b = payload[pos]
                pos += 1
                r = (b >> 4) & 0xF
                c = b & 0xF
                coords.append((r, c))
            cages.append((s, coords))
        return sol, cages

    def dump_puzzle_text(self, idx: int) -> str:
        sol, cages = self.read_puzzle_by_index(idx)
        out_lines = []
        # solution as 9 rows
        for r in range(9):
            row = sol[r * 9 : (r + 1) * 9]
            out_lines.append("[" + ", ".join(str(x) for x in row) + "]")
        out_lines.append("")
        # cages
        for s, coords in cages:
            coords_s = ",".join(f"({r},{c})" for (r, c) in coords)
            out_lines.append(f"{s} = {coords_s}")
        return "\n".join(out_lines)


# -------------------------
# CLI
# -------------------------
def build_parser():
    p = argparse.ArgumentParser(
        description="Pack Killer Sudoku ZIP into compact .ks files; inspect them."
    )
    p.add_argument(
        "zipfile", nargs="?", help="Input zip archive (required for packing)."
    )
    p.add_argument(
        "outdir",
        nargs="?",
        help="Output directory for .ks files (required for packing).",
    )
    p.add_argument(
        "--cage",
        type=int,
        action="append",
        help="Only pack this cage folder (can be repeated).",
    )
    p.add_argument("--inspect", help="Inspect an existing .ks file instead of packing.")
    p.add_argument(
        "--count", action="store_true", help="When inspecting, print puzzle count."
    )
    p.add_argument(
        "--show", type=int, help="When inspecting, show puzzle at this 0-based index."
    )
    p.add_argument("--quiet", action="store_true", help="Less verbose.")
    return p


def main(argv=None):
    args = build_parser().parse_args(argv)
    verbose = not args.quiet

    if args.inspect:
        reader = KSReader(args.inspect)
        if args.count:
            print(reader.puzzle_count())
        if args.show is not None:
            print(reader.dump_puzzle_text(args.show))
        reader.close()
        return

    if not args.zipfile or not args.outdir:
        print(
            "Error: packing requires ZIPFILE and OUTDIR (or use --inspect).",
            file=sys.stderr,
        )
        return

    packer = KSPacker(args.zipfile, args.outdir, verbose=verbose)
    cages = args.cage if args.cage else None
    packer.pack_all(cages=cages)


if __name__ == "__main__":
    main()
