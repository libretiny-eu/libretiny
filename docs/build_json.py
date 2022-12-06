import json

from ltchiptool import Board
from update_docs import board_obj_sort

boards = map(Board, Board.get_list())
boards = list(sorted(boards, key=board_obj_sort))
for i, board in enumerate(boards):
    boards[i] = dict(
        name=board.name,
        title=board.title,
        vendor=board.vendor,
        mcu=board.family.short_name,
    )

with open("boards.json", "w") as f:
    json.dump(boards, f)
