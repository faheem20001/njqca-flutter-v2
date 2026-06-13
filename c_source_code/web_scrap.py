from bs4 import BeautifulSoup
import json
import argparse
import re

from curl_cffi import requests as curl_requests


parser = argparse.ArgumentParser(description="Scrape Geekbench CPU results")
parser.add_argument("-url", "--url", required=True, help="Geekbench CPU result URL")
args = parser.parse_args()

URL = args.url.strip()

data = {
    "benchmark": "Geekbench 5 CPU",
    "url": URL
}


def extract_scores_from_text(text):
    text = re.sub(r"\s+", " ", text)

    # Top page format:
    # Geekbench 5 Score 1007 Single-Core Score 3156 Multi-Core Score
    m = re.search(
        r"Geekbench\s+5\s+Score\s+(\d+)\s+Single-Core\s+Score\s+(\d+)\s+Multi-Core\s+Score",
        text,
        re.IGNORECASE,
    )
    if m:
        return m.group(1), m.group(2)

    # Detailed performance section format
    single = re.search(
        r"Single-Core\s+Performance.*?Single-Core\s+Score\s+(\d+)",
        text,
        re.IGNORECASE | re.DOTALL,
    )
    multi = re.search(
        r"Multi-Core\s+Performance.*?Multi-Core\s+Score\s+(\d+)",
        text,
        re.IGNORECASE | re.DOTALL,
    )

    if single and multi:
        return single.group(1), multi.group(1)

    # Loose fallback
    loose = re.search(
        r"Single-Core\s+Score\s+(\d+).*?Multi-Core\s+Score\s+(\d+)",
        text,
        re.IGNORECASE | re.DOTALL,
    )
    if loose:
        return loose.group(1), loose.group(2)

    return None


try:
    response = curl_requests.get(
        URL,
        impersonate="chrome124",
        timeout=60,
    )
    response.raise_for_status()

    soup = BeautifulSoup(response.text, "html.parser")

    scores = None

    # Keep your previous selector support
    score_boxes = soup.select(".score")
    if len(score_boxes) >= 2:
        single = score_boxes[0].get_text(strip=True)
        multi = score_boxes[1].get_text(strip=True)
        if single.isdigit() and multi.isdigit():
            scores = (single, multi)

    # Text fallback
    if not scores:
        text = soup.get_text(" ", strip=True)
        scores = extract_scores_from_text(text)

    if scores:
        data["single_core_score"] = scores[0]
        data["multi_core_score"] = scores[1]
    else:
        data["score_parse_status"] = "FAILED"
        data["score_parse_error"] = "Score text not found"

except Exception as e:
    data["score_parse_status"] = "FAILED"
    data["score_parse_error"] = str(e)


print(json.dumps(data))