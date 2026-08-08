#!/usr/bin/env python3

import math
import subprocess
from dataclasses import dataclass
from datetime import datetime, timedelta
from pathlib import Path


@dataclass
class Commit:
    index: int
    commit_hash: str
    date: datetime
    subject: str
    insertions: int
    deletions: int
    files_changed: int
    lane: str


@dataclass
class SourceStats:
    source_files: int
    lines: int
    characters: int
    lines_by_subsystem: dict[str, int]


LANES = [
    "docs",
    "engine",
    "scene/ecs",
    "editor",
    "renderer/core",
    "renderer/model",
    "renderer/lights",
    "renderer/targets",
    "renderer/skybox",
    "renderer/shaders",
    "renderer/window",
    "renderer/camera",
    "assets",
    "other",
]

LANE_COLORS = {
    "docs": "#8b949e",
    "engine": "#58a6ff",
    "scene/ecs": "#a5d6ff",
    "editor": "#d2a8ff",
    "renderer/core": "#f78166",
    "renderer/model": "#ffab70",
    "renderer/lights": "#f2cc60",
    "renderer/targets": "#ff7b72",
    "renderer/skybox": "#79c0ff",
    "renderer/shaders": "#ffa657",
    "renderer/window": "#db6d28",
    "renderer/camera": "#bc8cff",
    "assets": "#7ee787",
    "other": "#6e7681",
}

SOURCE_EXTENSIONS = {".c", ".h", ".cpp", ".hpp"}


def is_source_file(path: Path) -> bool:
    if path.suffix not in SOURCE_EXTENSIONS:
        return False

    parts = path.parts

    if "external" in parts:
        return False

    if "build" in parts:
        return False

    return True


def classify_source_path(path: Path) -> str:
    path_text = str(path).lower()

    if "src/editor" in path_text:
        return "editor"

    if "src/engine" in path_text:
        return "engine"

    if "src/scene" in path_text or "src/ecs" in path_text:
        return "scene/ecs"

    if (
        "src/renderer/data_types/model" in path_text
        or "src/renderer/data_types/mesh" in path_text
        or "src/renderer/data_types/texture" in path_text
        or "src/renderer/data_types/material" in path_text
        or "src/renderer/data_types/vertex" in path_text
    ):
        return "renderer/model"

    if "src/renderer/data_types/lightobject" in path_text:
        return "renderer/lights"

    if "src/renderer/data_types/rendertarget" in path_text:
        return "renderer/targets"

    if "src/renderer/data_types/skybox" in path_text:
        return "renderer/skybox"

    if "src/renderer/shaders" in path_text:
        return "renderer/shaders"

    if "src/renderer/window" in path_text:
        return "renderer/window"

    if "src/renderer/camera" in path_text:
        return "renderer/camera"

    if "src/renderer/renderer" in path_text:
        return "renderer/core"

    if "src/renderer" in path_text:
        return "renderer/core"

    return "other"


def collect_source_stats(src_root: Path = Path("src")) -> SourceStats:
    source_file_count = 0
    total_lines = 0
    total_characters = 0
    lines_by_subsystem: dict[str, int] = {}

    for source_path in src_root.rglob("*"):
        if not source_path.is_file() or not is_source_file(source_path):
            continue

        source_text = source_path.read_text(encoding="utf-8", errors="replace")

        line_count = source_text.count("\n")
        if source_text and not source_text.endswith("\n"):
            line_count += 1

        source_file_count += 1
        total_lines += line_count
        total_characters += len(source_text)

        lane = classify_source_path(source_path)
        lines_by_subsystem[lane] = lines_by_subsystem.get(lane, 0) + line_count

    return SourceStats(
        source_files=source_file_count,
        lines=total_lines,
        characters=total_characters,
        lines_by_subsystem=dict(sorted(lines_by_subsystem.items())),
    )


def run_git_log(revision_range: str | None) -> str:
    command = [
        "git",
        "log",
        "--reverse",
        "--date=iso-strict",
        "--pretty=format:COMMIT%x09%h%x09%ad%x09%s",
        "--numstat",
    ]

    if revision_range is not None:
        command.append(revision_range)

    return subprocess.check_output(command, text=True)


def classify_commit(subject: str, files: list[str]) -> str:
    subject_lower = subject.lower()
    paths = " ".join(files).lower()

    if "src/editor" in paths or "editor" in subject_lower or "imgui" in subject_lower:
        return "editor"

    if "src/renderer/data_types/model" in paths or "src/renderer/data_types/mesh" in paths or "src/renderer/data_types/texture" in paths or "src/renderer/data_types/material" in paths or "src/renderer/data_types/vertex" in paths:
        return "renderer/model"

    if "src/renderer/data_types/lightobject" in paths:
        return "renderer/lights"

    if "src/renderer/data_types/rendertarget" in paths:
        return "renderer/targets"

    if "src/renderer/data_types/skybox" in paths:
        return "renderer/skybox"

    if "src/renderer/shaders" in paths or "shader" in subject_lower:
        return "renderer/shaders"

    if "src/renderer/window" in paths:
        return "renderer/window"

    if "src/renderer/camera" in paths:
        return "renderer/camera"

    if "src/renderer/renderer" in paths:
        return "renderer/core"

    if "src/renderer" in paths or "renderer" in subject_lower:
        return "renderer/core"
    
    if "src/renderer" in paths or "renderer" in subject_lower:
        return "renderer"

    if (
        "src/scene" in paths
        or "src/ecs" in paths
        or "scene" in subject_lower
        or "ecs" in subject_lower
    ):
        return "scene/ecs"

    if "src/engine" in paths or "engine" in subject_lower:
        return "engine"
    
    if "notes/" in paths or "readme" in paths or "docs" in subject_lower:
        return "docs"
    
    return "other"


def parse_commits(raw_log: str) -> list[Commit]:
    commits = []

    current_hash = None
    current_date = None
    current_subject = None
    insertions = 0
    deletions = 0
    files = []

    def flush():
        nonlocal current_hash, current_date, current_subject, insertions, deletions, files

        if current_hash is None:
            return

        lane = classify_commit(current_subject, files)

        commits.append(
            Commit(
                index=len(commits) + 1,
                commit_hash=current_hash,
                date=current_date,
                subject=current_subject,
                insertions=insertions,
                deletions=deletions,
                files_changed=len(files),
                lane=lane,
            )
        )

    for line in raw_log.splitlines():
        if line.startswith("COMMIT\t"):
            flush()

            _, current_hash, date_text, current_subject = line.split("\t", 3)
            current_date = datetime.fromisoformat(date_text)
            insertions = 0
            deletions = 0
            files = []
            continue

        parts = line.split("\t")
        if len(parts) == 3:
            added, removed, path = parts

            if added != "-":
                insertions += int(added)

            if removed != "-":
                deletions += int(removed)

            files.append(path)

    flush()

    return commits


def escape_svg(text: str) -> str:
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def commit_radius(commit: Commit) -> float:
    churn = commit.insertions + commit.deletions

    if churn <= 0:
        return 5.0

    return 5.0 + min(18.0, math.log(churn + 1) * 2.2)


def render_stats_card(parts: list[str], stats: SourceStats, width: int) -> None:
    card_x = width - 520
    card_y = 35
    card_w = 440
    card_h = 245

    parts.append(
        f'<rect x="{card_x}" y="{card_y}" width="{card_w}" height="{card_h}" '
        f'rx="10" fill="#161b22" stroke="#30363d" stroke-width="1"/>'
    )

    parts.append(
        f'<text x="{card_x + 18}" y="{card_y + 28}" '
        f'fill="#f0f6fc" font-family="monospace" font-size="14">'
        f'Commit 100 snapshot'
        f'</text>'
    )

    summary_lines = [
        f"source files: {stats.source_files}",
        f"lines:        {stats.lines}",
        f"characters:   {stats.characters}",
    ]

    for i, line in enumerate(summary_lines):
        parts.append(
            f'<text x="{card_x + 18}" y="{card_y + 55 + i * 18}" '
            f'fill="#c9d1d9" font-family="monospace" font-size="12">'
            f'{escape_svg(line)}'
            f'</text>'
        )

    parts.append(
        f'<text x="{card_x + 18}" y="{card_y + 118}" '
        f'fill="#8b949e" font-family="monospace" font-size="12">'
        f'source lines by lane'
        f'</text>'
    )

    lane_lines = [
        (lane, stats.lines_by_subsystem[lane])
        for lane in LANES
        if stats.lines_by_subsystem.get(lane, 0) > 0
    ]

    split_index = (len(lane_lines) + 1) // 2
    columns = [lane_lines[:split_index], lane_lines[split_index:]]
    column_x_offsets = [18, 232]
    value_x_offsets = [188, 402]

    for column_index, column in enumerate(columns):
        for row_index, (name, lines) in enumerate(column):
            y = card_y + 140 + row_index * 15
            label_x = card_x + column_x_offsets[column_index]
            value_x = card_x + value_x_offsets[column_index]

            parts.append(
                f'<text x="{label_x}" y="{y}" '
                f'fill="#c9d1d9" font-family="monospace" font-size="10">'
                f'{escape_svg(name + ":")}'
                f'</text>'
            )

            parts.append(
                f'<text x="{value_x}" y="{y}" text-anchor="end" '
                f'fill="#c9d1d9" font-family="monospace" font-size="10">'
                f'{lines}'
                f'</text>'
            )


def render_svg(commits: list[Commit], stats: SourceStats, output_path: Path) -> None:
    if not commits:
        raise RuntimeError("No commits found for the requested range.")

    width = 1400
    lane_height = 80
    margin_left = 150
    margin_right = 80
    margin_top = 310
    margin_bottom = 150

    height = margin_top + margin_bottom + lane_height * len(LANES)

    first_time = commits[0].date.timestamp()
    last_time = commits[-1].date.timestamp()
    time_span = max(1.0, last_time - first_time)

    def x_for(commit: Commit) -> float:
        t = (commit.date.timestamp() - first_time) / time_span
        return margin_left + t * (width - margin_left - margin_right)

    def y_for(lane: str) -> float:
        return margin_top + LANES.index(lane) * lane_height + lane_height * 0.5

    parts = []

    parts.append(
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">'
    )
    parts.append('<rect width="100%" height="100%" fill="#0d1117"/>')

    total_insertions = sum(commit.insertions for commit in commits)
    total_deletions = sum(commit.deletions for commit in commits)
    elapsed_days = max(1, (commits[-1].date.date() - commits[0].date.date()).days + 1)

    parts.append(
        f'<text x="{margin_left}" y="92" fill="#f0f6fc" font-family="monospace" font-size="32" font-weight="700">'
        f'Commit 100 Milestone Timeline'
        f'</text>'
    )

    parts.append(
        f'<text x="{margin_left}" y="121" fill="#8b949e" font-family="monospace" font-size="16">'
        f'{escape_svg(commits[0].date.date().isoformat())} → {escape_svg(commits[-1].date.date().isoformat())}'
        f'</text>'
    )

    parts.append(
        f'<text x="{margin_left}" y="151" fill="#c9d1d9" font-family="monospace" font-size="14">'
        f'{len(commits)} commits · {elapsed_days} days · +{total_insertions} / -{total_deletions}'
        f'</text>'
    )

    render_stats_card(parts, stats, width)

    for lane in LANES:
        y = y_for(lane)
        color = LANE_COLORS[lane]

        parts.append(
            f'<line x1="{margin_left}" y1="{y}" x2="{width - margin_right}" y2="{y}" '
            f'stroke="#30363d" stroke-width="1"/>'
        )
        parts.append(
            f'<text x="30" y="{y + 5}" fill="{color}" font-family="monospace" font-size="15">'
            f'{escape_svg(lane)}'
            f'</text>'
        )

    axis_y = height - margin_bottom + 35

    parts.append(
        f'<line x1="{margin_left}" y1="{axis_y}" '
        f'x2="{width - margin_right}" y2="{axis_y}" '
        f'stroke="#30363d" stroke-width="1"/>'
    )

    first_day = commits[0].date.date()
    last_day = commits[-1].date.date()

    # Start ticks on the Monday of the first commit's week.
    current_day = first_day - timedelta(days=first_day.weekday())

    while current_day <= last_day:
        day_start = datetime.combine(current_day, datetime.min.time()).replace(
            tzinfo=commits[0].date.tzinfo
        )
        day_timestamp = day_start.timestamp()

        t = (day_timestamp - first_time) / time_span
        x = margin_left + t * (width - margin_left - margin_right)

        if margin_left <= x <= width - margin_right:
            parts.append(
                f'<line x1="{x:.2f}" y1="{margin_top}" '
                f'x2="{x:.2f}" y2="{height - margin_bottom}" '
                f'stroke="#21262d" stroke-width="1"/>'
            )

            parts.append(
                f'<line x1="{x:.2f}" y1="{axis_y - 6}" '
                f'x2="{x:.2f}" y2="{axis_y + 6}" '
                f'stroke="#8b949e" stroke-width="1"/>'
            )

            label = current_day.strftime("%b %d")

            parts.append(
                f'<text x="{x:.2f}" y="{axis_y + 28}" '
                f'text-anchor="end" fill="#8b949e" '
                f'font-family="monospace" font-size="10" '
                f'transform="rotate(-35 {x:.2f} {axis_y + 28})">'
                f'{escape_svg(label)}'
                f'</text>'
            )

        current_day += timedelta(days=7)

    for commit in commits:
        x = x_for(commit)
        y = y_for(commit.lane)
        r = commit_radius(commit)
        color = LANE_COLORS[commit.lane]
        churn = commit.insertions + commit.deletions

        title = (
            f"{commit.commit_hash}\\n"
            f"{commit.date.date().isoformat()}\\n"
            f"{commit.subject}\\n"
            f"+{commit.insertions} -{commit.deletions}\\n"
            f"{commit.files_changed} files"
        )

        parts.append(
            f'<circle cx="{x:.2f}" cy="{y:.2f}" r="{r:.2f}" fill="{color}" opacity="0.85">'
            f'<title>{escape_svg(title)}</title>'
            f'</circle>'
        )

        show_label = commit.index == 1 or commit.index % 5 == 0 or commit.index == len(commits)

        if show_label:
            parts.append(
                f'<text x="{x:.2f}" y="{y - r - 8:.2f}" text-anchor="middle" '
                f'fill="#c9d1d9" font-family="monospace" font-size="10">'
                f'{commit.index}'
                f'</text>'
            )
                
    parts.append(
        f'<text x="{margin_left}" y="{height - 28}" '
        f'fill="#8b949e" font-family="monospace" font-size="12">'
        f'Labeled bubbles mark every 5th commit. Bubble size = churn. Color = inferred subsystem.'
        f'</text>'
    )

    parts.append("</svg>")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(parts), encoding="utf-8")


def main() -> None:
    revision_range = None
    output_path = Path("notes/commit-100-timeline.svg")

    raw_log = run_git_log(revision_range)
    commits = parse_commits(raw_log)
    stats = collect_source_stats()
    render_svg(commits, stats, output_path)

    print(f"wrote: {output_path}")
    print(f"commits: {len(commits)}")
    print(f"source files: {stats.source_files}")
    print(f"lines: {stats.lines}")
    print(f"characters: {stats.characters}")


if __name__ == "__main__":
    main()