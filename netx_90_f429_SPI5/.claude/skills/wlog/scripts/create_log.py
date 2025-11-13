#!/usr/bin/env python3
"""
작업 로그 파일을 생성하는 스크립트
일시가 포함된 파일명으로 Markdown 형식의 작업 로그를 생성합니다.
"""

import sys
from datetime import datetime
from pathlib import Path


def create_work_log(
    content: str,
    output_dir: str = ".",
    prefix: str = "work_log",
    date_format: str = "%Y%m%d_%H%M%S"
) -> str:
    """
    작업 로그 파일을 생성합니다.
    
    Args:
        content: 작업 로그 내용 (Markdown 형식)
        output_dir: 출력 디렉토리 경로
        prefix: 파일명 접두사 (기본값: "work_log")
        date_format: 날짜 형식 (기본값: "%Y%m%d_%H%M%S")
    
    Returns:
        생성된 파일의 전체 경로
    """
    # 현재 시각으로 파일명 생성
    timestamp = datetime.now().strftime(date_format)
    filename = f"{timestamp}_{prefix}.md"
    
    # 출력 디렉토리 생성
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    
    # 전체 파일 경로
    filepath = output_path / filename
    
    # 파일 작성
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    
    return str(filepath)


def main():
    if len(sys.argv) < 2:
        print("Usage: python create_log.py <content> [output_dir] [prefix] [date_format]")
        print("\nExample:")
        print('  python create_log.py "# Work Log\\n\\n## Tasks\\n- Task 1" ./logs')
        sys.exit(1)
    
    content = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "."
    prefix = sys.argv[3] if len(sys.argv) > 3 else "work_log"
    date_format = sys.argv[4] if len(sys.argv) > 4 else "%Y%m%d_%H%M%S"
    
    filepath = create_work_log(content, output_dir, prefix, date_format)
    print(f"Work log created: {filepath}")


if __name__ == "__main__":
    main()
