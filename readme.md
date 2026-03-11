<img width="1600" height="925" alt="chessex" src="https://github.com/user-attachments/assets/ecc747c6-be46-43ed-9503-2c3898c0ca6a" />

PART 2:

<img width="706" height="652" alt="part1chess" src="https://github.com/user-attachments/assets/36ab1404-5332-4952-97a5-8e9b75a43eed" />

For part 2, the pawns, knights, and kings logic was implemented. This was done via a bitboard. It is storing the pieces based on their color and piece type. It is using bit shifts to handle the pieces moving. There are various other things like masking and validating the move. The UI updates after the move is done. 

Part 3:
Initially I started out by creating functions similar to how I did the previous three pieces. But, then I took a look at the magic bitboard which had functions I could use for these sliding pieces. So I called those functions in my chess script and made sure the new pieces were added to the occupancy function. 

Part 4:
The code went through a few iterations. Originally, I was mixing state and bitboard readings when handling what the AI reads on the board. But, I switched fully to bitboard readings which ended up simplifying my code and is just faster. 
Another was managing the move simulation so the board returns to its correct state after searching. By copying the pieces beforehand, it allowed them to be restored. Originally I stored it via the string but switched to memspcy as I could use memory due to having the pieces stored in _pieces.
The AI reaches depth 3. The AI originally wasn’t great, but with the addition of the pieceTables, it now works pretty well. Now granted I’m bad at Chess, but it beats me everytime. 

https://github.com/user-attachments/assets/871e2a70-4643-45d0-861c-6d382e46c934

