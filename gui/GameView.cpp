#include "GameView.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "HexLayout.h"
#include "InputController.h"
#include "hive/Hex.h"

namespace
{

constexpr std::array<PieceType, 8> kHandOrder = {
    PieceType::Queen,       PieceType::Spider, PieceType::Ant,     PieceType::Mosquito,
    PieceType::Grasshopper, PieceType::Beetle, PieceType::Ladybug, PieceType::Pillbug,
};

std::string pieceAssetPath(PieceType type)
{
    switch (type)
    {
        case PieceType::Queen:
            return "assets/pieces/queen.png";
        case PieceType::Spider:
            return "assets/pieces/spider.png";
        case PieceType::Beetle:
            return "assets/pieces/beetle.png";
        case PieceType::Grasshopper:
            return "assets/pieces/grasshopper.png";
        case PieceType::Ant:
            return "assets/pieces/ant.png";
        case PieceType::Mosquito:
            return "assets/pieces/mosquito.png";
        case PieceType::Ladybug:
            return "assets/pieces/ladybug.png";
        case PieceType::Pillbug:
            return "assets/pieces/pillbug.png";
    }
    throw std::logic_error("unhandled PieceType");
}

// The exported glyphs are black-on-transparent (easiest to eyeball in
// Finder); rewriting RGB to white while preserving alpha lets
// sf::Sprite::setColor() tint them to either player's ink color, since
// SFML tints by multiplying texture color -- a black source could never
// be lightened back up for the dark player's pale ink.
sf::Texture loadWhitenedTexture(const std::string &path)
{
    sf::Image image;
    if (!image.loadFromFile(path))
    {
        throw std::runtime_error("failed to load " + path + " -- run ./hive from the repo root");
    }
    const sf::Vector2u size = image.getSize();
    for (unsigned y = 0; y < size.y; ++y)
    {
        for (unsigned x = 0; x < size.x; ++x)
        {
            const sf::Color pixel = image.getPixel(x, y);
            image.setPixel(x, y, sf::Color(255, 255, 255, pixel.a));
        }
    }
    sf::Texture texture;
    if (!texture.loadFromImage(image))
    {
        throw std::runtime_error("failed to upload texture for " + path);
    }
    return texture;
}

// Player colors: light/dark hex fill, ink tinted to the opposite end of
// the scale for contrast against its own tile.
// Not constexpr: sf::Color's constructor isn't a constexpr function in
// SFML 2.6 (it became one in 3.x). const is enough to keep these
// internal to this anonymous namespace.
const sf::Color kWhiteTile(224, 196, 146);
const sf::Color kWhiteInk(45, 33, 22);
const sf::Color kBlackTile(46, 43, 39);
const sf::Color kBlackInk(235, 228, 215);

// Shared between drawHands (panel extent) and drawBoard's fitLayout (how
// much width is left over for the board once both panels are cut out),
// so the two never disagree about where the board's drawing area starts
// and ends.
constexpr float kPanelWidth = 190.f;

// Shared between drawStatus (bar height) and fitLayout (how much height
// is left below it) for the same reason. Currently holds just the turn
// indicator, but is the anchor for the InputController-state indicator
// and cancel affordance planned once clicking exists -- a real top bar
// now instead of floating text, so those can be added without another
// layout reshuffle.
constexpr float kTopBarHeight = 64.f;

// Hand-panel row geometry at full size. Both are scaled down together by
// fitHandLayout below on a window too short for them, and every consumer
// goes through that -- drawHands (drawing the rows) and hitTest
// (hit-testing clicks against those same rows) so the two can never
// disagree about where a row actually is.
constexpr float kSwatchRadius = 34.f;
constexpr float kRowSpacing = 96.f;
// The first swatch's top edge must clear the top bar (kTopBarHeight=64)
// plus its 3px divider, so it doesn't poke back up into the bar, plus
// some breathing room.
constexpr float kHandTopGap = kTopBarHeight + 3.f + 20.f;
constexpr float kHandBottomMargin = 20.f;
constexpr float kSideMargin = 90.f;

// Row pitch and swatch radius actually used at this window height, plus
// the resulting first-row center.
//
// At full size the eight rows need kHandTopGap + 7*kRowSpacing +
// 2*kSwatchRadius = 827px of height. The course dev container's VNC
// desktop is only 768 tall, so the last row (Pillbug) ran from y=759 to
// y=827 and all that showed of it was a 9px sliver at the bottom edge --
// the reported "cut off at the bottom". Raising VNC_RESOLUTION would not
// have fixed this for good: any display shorter than ~850px hits it,
// including a plain 1366x768 laptop running the game natively.
//
// So scale the pitch and the swatch down together to whatever height
// there is, the same always-fits approach fitLayout takes for the board.
// Never scales *up* past the constants above, so a tall display renders
// exactly as it did before.
struct HandLayout
{
    float rowSpacing;
    float swatchRadius;
    float topMargin;  // center of the first row
};

HandLayout fitHandLayout(float windowHeight)
{
    constexpr float kNaturalExtent =
        static_cast<float>(kHandOrder.size() - 1) * kRowSpacing + 2.f * kSwatchRadius;

    const float available = windowHeight - kHandTopGap - kHandBottomMargin;
    // Clamped rather than just min'd: a window too short even for the
    // floor would otherwise produce zero or negative radii and draw
    // nothing at all, which is a worse failure than rows that overlap.
    const float scale = std::clamp(available / kNaturalExtent, 0.25f, 1.f);

    const float swatchRadius = kSwatchRadius * scale;
    return HandLayout{kRowSpacing * scale, swatchRadius, kHandTopGap + swatchRadius};
}

// Action-menu button geometry, shared between drawActionMenu (drawing
// the row) and hitTest (hit-testing clicks against it), same reasoning
// as the hand-panel constants above.
constexpr float kActionButtonRadius = 22.f;
constexpr float kActionButtonSpacing = 64.f;
// Right-aligned in the bar, clear of the side panel, so the row grows
// leftward as more buttons (e.g. Move + Throw) are shown without ever
// crowding the centered turn label.
constexpr float kActionButtonRightMargin = kPanelWidth + 90.f;

std::string actionKindLabel(ActionKind kind)
{
    switch (kind)
    {
        case ActionKind::Move:
            return "Move";
        case ActionKind::Throw:
            return "Throw";
    }
    throw std::logic_error("unhandled ActionKind");
}

sf::Vector2f actionButtonCenter(std::size_t index, float windowWidth)
{
    const float rightmostX = windowWidth - kActionButtonRightMargin;
    return {rightmostX - static_cast<float>(index) * kActionButtonSpacing, kTopBarHeight / 2.f};
}

// Imitation-popup icon geometry, shared between drawImitationPopup
// (drawing the row) and hitTest (hit-testing clicks against it). Drawn
// centered below the bar rather than inside it (contrast the
// kActionButtonRadius row): "drop down from the bar," per the original
// design, needs its own room a fixed 64px bar doesn't have.
constexpr float kImitationIconRadius = 26.f;
constexpr float kImitationIconSpacing = 64.f;
constexpr float kImitationPopupTop = kTopBarHeight + 14.f;

sf::Vector2f imitationIconCenter(std::size_t index, std::size_t count, float windowWidth)
{
    const float totalWidth = static_cast<float>(count - 1) * kImitationIconSpacing;
    const float startX = windowWidth / 2.f - totalWidth / 2.f;
    return {startX + static_cast<float>(index) * kImitationIconSpacing, kImitationPopupTop + kImitationIconRadius};
}

// Chooses a hex size and pixel origin so every occupied hex fits inside
// the area between the two hand panels and below the top bar, centered,
// no matter how far the hive has grown -- an always-fits auto-fit instead
// of drag/pan, which would otherwise need to be disambiguated from
// click-to-select in the upcoming InputController.
HexLayout fitLayout(const std::vector<Hex> &occupied, float windowWidth, float windowHeight)
{
    constexpr float kMinSize = 18.f;
    constexpr float kMaxSize = 70.f;
    constexpr float kBoardMargin = 40.f;

    const float availableWidth = windowWidth - 2.f * (kPanelWidth + kBoardMargin);
    const float availableTop = kTopBarHeight + kBoardMargin;
    const float availableBottom = windowHeight - kBoardMargin;
    const float availableHeight = availableBottom - availableTop;
    const sf::Vector2f areaCenter{windowWidth / 2.f, (availableTop + availableBottom) / 2.f};

    if (occupied.empty())
    {
        return HexLayout(kMaxSize, areaCenter);
    }

    // Bounding box of hex centers, in unit-size (size=1) pixel space.
    float minUx = 0.f;
    float maxUx = 0.f;
    float minUy = 0.f;
    float maxUy = 0.f;
    for (std::size_t i = 0; i < occupied.size(); ++i)
    {
        const float q = static_cast<float>(occupied[i].q);
        const float r = static_cast<float>(occupied[i].r);
        const float ux = 1.5f * q;
        const float uy = std::sqrt(3.f) * 0.5f * q + std::sqrt(3.f) * r;
        if (i == 0)
        {
            minUx = maxUx = ux;
            minUy = maxUy = uy;
        }
        else
        {
            minUx = std::min(minUx, ux);
            maxUx = std::max(maxUx, ux);
            minUy = std::min(minUy, uy);
            maxUy = std::max(maxUy, uy);
        }
    }

    // Pad the bounding box by each hex's own footprint (flat-top: 2*size
    // wide vertex-to-vertex, sqrt(3)*size tall flat-to-flat) so tiles at
    // the edge aren't clipped, not just their centers.
    const float totalUnitWidth = (maxUx - minUx) + 2.f;
    const float totalUnitHeight = (maxUy - minUy) + std::sqrt(3.f);

    float size = std::min(availableWidth / totalUnitWidth, availableHeight / totalUnitHeight);
    size = std::clamp(size, kMinSize, kMaxSize);

    const float centerUx = (minUx + maxUx) / 2.f;
    const float centerUy = (minUy + maxUy) / 2.f;
    const sf::Vector2f origin{areaCenter.x - size * centerUx, areaCenter.y - size * centerUy};

    return HexLayout(size, origin);
}

}  // namespace

GameView::GameView()
{
    for (PieceType type : kHandOrder)
    {
        pieceTextures_.emplace(type, loadWhitenedTexture(pieceAssetPath(type)));
    }
    if (!font_.loadFromFile("assets/fonts/LiberationSans-Regular.ttf"))
    {
        throw std::runtime_error("failed to load font -- run ./hive from the repo root");
    }
}

void GameView::drawHands(sf::RenderTarget &target, const Game &game) const
{
    constexpr float kDividerWidth = 3.f;

    const sf::Vector2u windowSize = target.getSize();
    const float windowWidth = static_cast<float>(windowSize.x);
    const float windowHeight = static_cast<float>(windowSize.y);
    const HandLayout hand = fitHandLayout(windowHeight);

    for (Color color : {Color::White, Color::Black})
    {
        const bool isWhite = color == Color::White;
        const sf::Color tileColor = isWhite ? kWhiteTile : kBlackTile;
        const float x = isWhite ? kSideMargin : windowWidth - kSideMargin;
        const Player &player = game.player(color);

        // A real panel bar behind the whole column, not just a thin ring
        // around each glyph -- what "two vertical bars" actually implies.
        // Starts below the top bar (drawStatus draws it opaque, on top of
        // this) rather than under it, so the two don't visually overlap.
        const float panelHeight = windowHeight - kTopBarHeight;
        sf::RectangleShape panel({kPanelWidth, panelHeight});
        panel.setPosition({isWhite ? 0.f : windowWidth - kPanelWidth, kTopBarHeight});
        panel.setFillColor(sf::Color(tileColor.r, tileColor.g, tileColor.b, 90));
        target.draw(panel);

        // A crisp line at the panel's inner edge, so the board area reads
        // as a distinct region instead of just fading into the panel.
        sf::RectangleShape divider({kDividerWidth, panelHeight});
        divider.setPosition({isWhite ? kPanelWidth : windowWidth - kPanelWidth - kDividerWidth, kTopBarHeight});
        divider.setFillColor(sf::Color(90, 88, 82));
        target.draw(divider);

        for (std::size_t i = 0; i < kHandOrder.size(); ++i)
        {
            const PieceType type = kHandOrder[i];
            const float y = hand.topMargin + static_cast<float>(i) * hand.rowSpacing;

            sf::CircleShape hex(hand.swatchRadius, 6);
            hex.setOrigin({hand.swatchRadius, hand.swatchRadius});
            hex.setRotation(30.f);
            hex.setPosition({x, y});
            hex.setFillColor(tileColor);
            hex.setOutlineColor(sf::Color(90, 88, 82));
            // Negative thickness draws the outline inward (see
            // GameView::drawBoard) -- irrelevant here since hand swatches
            // never sit edge-to-edge with another hex, but kept
            // consistent so every hex-outline in this file behaves the
            // same way.
            hex.setOutlineThickness(-2.f);
            target.draw(hex);

            drawPiece(target, isWhite ? kWhiteInk : kBlackInk, type, {x, y}, hand.swatchRadius);

            // White text unreadable against White's own pale tan panel --
            // use each side's ink color (already used to tint the glyph)
            // instead of a single hardcoded color for both.
            // Character size and the gap to the swatch both track the
            // scaled radius, so a shrunk row stays proportioned instead
            // of a full-size count crowding a smaller hex.
            const float labelScale = hand.swatchRadius / kSwatchRadius;
            const unsigned labelSize = static_cast<unsigned>(std::max(12.f, 22.f * labelScale));
            const float labelGap = 12.f * labelScale;
            sf::Text label("x" + std::to_string(player.remaining(type)), font_, labelSize);
            label.setFillColor(isWhite ? kWhiteInk : kBlackInk);
            const float labelWidth = label.getLocalBounds().width;
            const float labelX =
                isWhite ? x + hand.swatchRadius + labelGap : x - hand.swatchRadius - labelGap - labelWidth;
            label.setPosition({labelX, y - static_cast<float>(labelSize) / 2.f});
            target.draw(label);
        }
    }
}

void GameView::drawPiece(sf::RenderTarget &target, sf::Color tint, PieceType type, sf::Vector2f center, float hexSize) const
{
    const sf::Texture &texture = pieceTextures_.at(type);
    const sf::Vector2u textureSize = texture.getSize();
    sf::Sprite sprite(texture);
    const float scale = (hexSize * 1.5f) / static_cast<float>(textureSize.x);
    sprite.setScale({scale, scale});
    sprite.setOrigin({static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f});
    sprite.setPosition(center);
    sprite.setColor(tint);
    target.draw(sprite);
}

void GameView::drawBoard(sf::RenderTarget &target, const Game &game) const
{
    const sf::Vector2u windowSize = target.getSize();
    const Board &board = game.board();
    const std::vector<Hex> occupied = board.occupiedHexes();
    const HexLayout layout = fitLayout(occupied, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    const float hexSize = layout.size();

    for (const Hex &hex : occupied)
    {
        const Piece *piece = board.topAt(hex);
        const bool isWhite = piece->color == Color::White;
        const sf::Vector2f center = layout.hexToPixel(hex);

        sf::CircleShape tile(hexSize, 6);
        tile.setOrigin({hexSize, hexSize});
        tile.setRotation(30.f);
        tile.setPosition(center);
        tile.setFillColor(isWhite ? kWhiteTile : kBlackTile);
        tile.setOutlineColor(sf::Color(90, 88, 82));
        // Negative thickness draws the outline inward instead of SFML's
        // default outward -- with a positive thickness, adjacent tiles'
        // outlines (each independently sized to exactly `hexSize`, which
        // is also what hexToPixel spaces them by) bled 2px past their
        // shared edge into the neighboring tile, making seams between
        // occupied hexes look misaligned/doubled.
        tile.setOutlineThickness(-2.f);
        target.draw(tile);

        drawPiece(target, isWhite ? kWhiteInk : kBlackInk, piece->type, center, hexSize);
    }

    // A second pass, strictly after every tile+piece above: the badge
    // sits offset toward a hex's corner, which is shared ground with
    // 2-3 neighboring hexes, so drawing it inline (in the loop above)
    // meant its visibility depended on occupiedHexes()'s iteration
    // order (backed by an unordered_map, so unspecified) -- a neighbor
    // drawn afterward could paint its own opaque tile right over the
    // badge. Drawing every badge only after every tile/piece guarantees
    // badges always end up on top, regardless of hex order.
    for (const Hex &hex : occupied)
    {
        const std::size_t height = board.stackHeight(hex);
        if (height <= 1)
        {
            continue;
        }

        const Piece *piece = board.topAt(hex);
        const sf::Vector2f center = layout.hexToPixel(hex);
        const float badgeRadius = hexSize * 0.3f;
        const float badgeOffset = hexSize * 0.55f;
        const sf::Vector2f badgeCenter = center + sf::Vector2f(badgeOffset, badgeOffset);

        sf::CircleShape badge(badgeRadius);
        badge.setOrigin({badgeRadius, badgeRadius});
        badge.setPosition(badgeCenter);
        badge.setFillColor(sf::Color(178, 48, 41));
        badge.setOutlineColor(sf::Color::White);
        badge.setOutlineThickness(1.5f);
        target.draw(badge);

        const unsigned int badgeTextSize = std::max(12u, static_cast<unsigned int>(badgeRadius * 1.15f));
        sf::Text badgeText(std::to_string(height), font_, badgeTextSize);
        badgeText.setFillColor(sf::Color::White);
        const sf::FloatRect bounds = badgeText.getLocalBounds();
        badgeText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        badgeText.setPosition(badgeCenter);
        target.draw(badgeText);

        // An elevated Mosquito moves using Beetle's own pattern, not a
        // fresh imitation choice, until it comes back down (see
        // Game::syncMosquitoStrategy) -- shown here, unconditionally,
        // rather than only while selected, since "this piece is
        // currently a Beetle" is a fact about the board, not about
        // whose turn it is or what's clicked. This can never collide
        // with drawSelection's currentImitation() badge: that one never
        // fires for an elevated Mosquito in the first place (Game::
        // addMosquitoMoves's elevated branch emits plain Movement, so
        // imitationOptions() is always empty for it) -- same "top",
        // same red-badge look, because they're really the same kind of
        // indicator, just sourced differently (persistent board state
        // here vs. an in-progress selection there).
        if (piece->type == PieceType::Mosquito)
        {
            const float beetleBadgeRadius = hexSize * 0.32f;
            const float beetleBadgeOffset = hexSize * 0.55f;
            const sf::Vector2f beetleBadgeCenter = center - sf::Vector2f(0.f, beetleBadgeOffset);

            sf::CircleShape beetleBadge(beetleBadgeRadius);
            beetleBadge.setOrigin({beetleBadgeRadius, beetleBadgeRadius});
            beetleBadge.setPosition(beetleBadgeCenter);
            beetleBadge.setFillColor(sf::Color(178, 48, 41));
            beetleBadge.setOutlineColor(sf::Color::White);
            beetleBadge.setOutlineThickness(1.5f);
            target.draw(beetleBadge);

            drawPiece(target, sf::Color::White, PieceType::Beetle, beetleBadgeCenter, beetleBadgeRadius);
        }
    }
}

void GameView::drawSelection(sf::RenderTarget &target, const Game &game, const InputController &controller) const
{
    const sf::Vector2u windowSize = target.getSize();
    const std::vector<Hex> occupied = game.board().occupiedHexes();
    const HexLayout layout = fitLayout(occupied, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    const float hexSize = layout.size();

    if (const std::optional<Hex> selected = controller.selectedHex())
    {
        sf::CircleShape ring(hexSize, 6);
        ring.setOrigin({hexSize, hexSize});
        ring.setRotation(30.f);
        ring.setPosition(layout.hexToPixel(*selected));
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(sf::Color(255, 205, 60));
        // Inward, like drawBoard's tile outline -- a positive thickness
        // here bled the ring past the selected hex's true edge and onto
        // whichever neighbor happened to be drawn next, reading as if
        // the border belonged to that piece instead.
        ring.setOutlineThickness(-4.f);
        target.draw(ring);

        // The Mosquito's "icon on current imitation target" from the
        // original design -- same red-badge language as drawBoard's
        // stack-height badge (not a player-tile swatch), so the two
        // read as the same kind of thing: a small overlay marking extra
        // state on top of the piece, not a second piece. Offset
        // straight up rather than toward a corner: drawBoard's stack
        // badges always sit toward a hex's bottom-right, so "up" keeps
        // this clear of every neighboring hex's own stack badge too,
        // not just this hex's (a diagonal offset shares ground with
        // 2-3 neighbors -- see the seam-bleed comment on drawBoard's
        // tile outline above). Drawn here, after drawBoard has already
        // finished both of its passes (see main.cpp's render order),
        // so it's never at risk of being painted over by a neighboring
        // tile the way an inline-drawn badge was (see drawBoard's
        // two-pass comment) -- this is already a second pass, later
        // than drawBoard's own.
        if (const std::optional<PieceType> imitating = controller.currentImitation())
        {
            const float badgeRadius = hexSize * 0.32f;
            const float badgeOffset = hexSize * 0.55f;
            const sf::Vector2f badgeCenter = layout.hexToPixel(*selected) - sf::Vector2f(0.f, badgeOffset);

            sf::CircleShape swatch(badgeRadius);
            swatch.setOrigin({badgeRadius, badgeRadius});
            swatch.setPosition(badgeCenter);
            swatch.setFillColor(sf::Color(178, 48, 41));
            swatch.setOutlineColor(sf::Color::White);
            swatch.setOutlineThickness(1.5f);
            target.draw(swatch);

            drawPiece(target, sf::Color::White, *imitating, badgeCenter, badgeRadius);
        }
    }

    // Same hex-bordered look as the selection ring above (just green,
    // not gold) rather than a small dot -- a dot doesn't read as "this
    // whole hex is a legal target," especially for an empty destination
    // that otherwise has no tile drawn there at all (drawBoard only
    // draws occupied hexes).
    for (const Hex &hex : controller.highlightedHexes())
    {
        sf::CircleShape marker(hexSize, 6);
        marker.setOrigin({hexSize, hexSize});
        marker.setRotation(30.f);
        marker.setPosition(layout.hexToPixel(hex));
        marker.setFillColor(sf::Color::Transparent);
        marker.setOutlineColor(sf::Color(90, 200, 120, 230));
        marker.setOutlineThickness(-4.f);
        target.draw(marker);
    }

    // A hand-originated Placement has no board hex to ring (selectedHex()
    // is nullopt for it -- see AwaitingDestination.h), so without this
    // there was no visual sign at all of which piece a click on the hand
    // panel had actually picked up. Ringing that same row mirrors the
    // board selection ring above, just anchored to the panel instead.
    if (const std::optional<PieceType> placing = controller.selectedHandPieceType())
    {
        const bool isWhite = game.turn() == Color::White;
        const float x = isWhite ? kSideMargin : static_cast<float>(windowSize.x) - kSideMargin;
        for (std::size_t i = 0; i < kHandOrder.size(); ++i)
        {
            if (kHandOrder[i] != *placing)
            {
                continue;
            }
            const HandLayout hand = fitHandLayout(static_cast<float>(windowSize.y));
            const float y = hand.topMargin + static_cast<float>(i) * hand.rowSpacing;
            sf::CircleShape ring(hand.swatchRadius, 6);
            ring.setOrigin({hand.swatchRadius, hand.swatchRadius});
            ring.setRotation(30.f);
            ring.setPosition({x, y});
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color(255, 205, 60));
            ring.setOutlineThickness(-4.f);
            target.draw(ring);
            break;
        }
    }
}

void GameView::drawStatus(sf::RenderTarget &target, const Game &game) const
{
    const sf::Vector2u windowSize = target.getSize();
    const float windowWidth = static_cast<float>(windowSize.x);
    const float windowHeight = static_cast<float>(windowSize.y);

    // A real bar, not floating text -- this is the anchor for the
    // InputController-state indicator and cancel affordance that land
    // alongside the turn label once clicking exists.
    sf::RectangleShape topBar({windowWidth, kTopBarHeight});
    topBar.setFillColor(sf::Color(66, 62, 54));
    target.draw(topBar);

    sf::RectangleShape topBarDivider({windowWidth, 3.f});
    topBarDivider.setPosition({0.f, kTopBarHeight});
    topBarDivider.setFillColor(sf::Color(40, 38, 34));
    target.draw(topBarDivider);

    const std::string turnLabel = (game.turn() == Color::White ? std::string("White") : std::string("Black")) +
                                   "'s turn -- Turn " + std::to_string(game.turnNumber());
    sf::Text turnText(turnLabel, font_, 24);
    turnText.setFillColor(sf::Color(235, 228, 210));
    const sf::FloatRect turnBounds = turnText.getLocalBounds();
    turnText.setOrigin(turnBounds.left + turnBounds.width / 2.f, turnBounds.top + turnBounds.height / 2.f);
    turnText.setPosition({windowWidth / 2.f, kTopBarHeight / 2.f});
    target.draw(turnText);

    if (game.status() == GameStatus::Ongoing)
    {
        return;
    }

    std::string bannerLabel;
    switch (game.status())
    {
        case GameStatus::WhiteWins:
            bannerLabel = "White wins!";
            break;
        case GameStatus::BlackWins:
            bannerLabel = "Black wins!";
            break;
        case GameStatus::Draw:
            bannerLabel = "Draw!";
            break;
        case GameStatus::Ongoing:
            return; // unreachable -- handled above
    }

    sf::RectangleShape overlay({windowWidth, windowHeight});
    overlay.setFillColor(sf::Color(0, 0, 0, 140));
    target.draw(overlay);

    sf::Text banner(bannerLabel, font_, 64);
    banner.setFillColor(sf::Color::White);
    const sf::FloatRect bannerBounds = banner.getLocalBounds();
    banner.setOrigin(bannerBounds.left + bannerBounds.width / 2.f, bannerBounds.top + bannerBounds.height / 2.f);
    banner.setPosition({windowWidth / 2.f, windowHeight / 2.f});
    target.draw(banner);
}

void GameView::drawActionMenu(sf::RenderTarget &target, const InputController &controller) const
{
    const std::vector<ActionKind> options = controller.actionOptions();
    if (options.empty())
    {
        return;
    }

    const float windowWidth = static_cast<float>(target.getSize().x);
    for (std::size_t i = 0; i < options.size(); ++i)
    {
        const sf::Vector2f center = actionButtonCenter(i, windowWidth);

        sf::CircleShape button(kActionButtonRadius);
        button.setOrigin({kActionButtonRadius, kActionButtonRadius});
        button.setPosition(center);
        button.setFillColor(sf::Color(90, 86, 76));
        button.setOutlineColor(sf::Color(235, 228, 210));
        button.setOutlineThickness(-2.f);
        target.draw(button);

        sf::Text label(actionKindLabel(options[i]), font_, 14);
        label.setFillColor(sf::Color(235, 228, 210));
        const sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        label.setPosition(center);
        target.draw(label);
    }
}

void GameView::drawCancelButton(sf::RenderTarget &target, const InputController &controller) const
{
    if (!controller.requiresExplicitCancel())
    {
        return;
    }

    const float windowWidth = static_cast<float>(target.getSize().x);
    const sf::Vector2f center = actionButtonCenter(0, windowWidth);

    sf::CircleShape button(kActionButtonRadius);
    button.setOrigin({kActionButtonRadius, kActionButtonRadius});
    button.setPosition(center);
    // A distinct, warm color (contrast the Move/Throw buttons' neutral
    // gray) so the one always-available way out of a throw-in-progress
    // reads as deliberately different from an ordinary action choice.
    button.setFillColor(sf::Color(150, 60, 50));
    button.setOutlineColor(sf::Color(235, 228, 210));
    button.setOutlineThickness(-2.f);
    target.draw(button);

    sf::Text label("Cancel", font_, 12);
    label.setFillColor(sf::Color(235, 228, 210));
    const sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    label.setPosition(center);
    target.draw(label);
}

void GameView::drawImitationPopup(sf::RenderTarget &target, const Game &game, const InputController &controller) const
{
    const std::vector<PieceType> options = controller.imitationOptions();
    if (options.empty())
    {
        return;
    }

    const float windowWidth = static_cast<float>(target.getSize().x);
    const std::optional<PieceType> current = controller.currentImitation();

    // A backing panel distinct from the top bar itself, so this clearly
    // reads as a dropdown rather than part of the fixed status bar.
    const float panelWidth =
        static_cast<float>(options.size() - 1) * kImitationIconSpacing + kImitationIconRadius * 2.f + 24.f;
    const float panelHeight = kImitationIconRadius * 2.f + 20.f;
    sf::RectangleShape panel({panelWidth, panelHeight});
    panel.setOrigin({panelWidth / 2.f, 0.f});
    panel.setPosition({windowWidth / 2.f, kImitationPopupTop - 10.f});
    panel.setFillColor(sf::Color(66, 62, 54, 235));
    panel.setOutlineColor(sf::Color(40, 38, 34));
    panel.setOutlineThickness(-2.f);
    target.draw(panel);

    for (std::size_t i = 0; i < options.size(); ++i)
    {
        const sf::Vector2f center = imitationIconCenter(i, options.size(), windowWidth);

        sf::CircleShape swatch(kImitationIconRadius, 6);
        swatch.setOrigin({kImitationIconRadius, kImitationIconRadius});
        swatch.setRotation(30.f);
        swatch.setPosition(center);
        swatch.setFillColor(game.turn() == Color::White ? kWhiteTile : kBlackTile);
        // Gold outline on whichever icon matches currentImitation() --
        // same "this one's picked" language as every other selection
        // indicator in this file.
        swatch.setOutlineColor(current == options[i] ? sf::Color(255, 205, 60) : sf::Color(90, 88, 82));
        swatch.setOutlineThickness(-3.f);
        target.draw(swatch);

        drawPiece(target, game.turn() == Color::White ? kWhiteInk : kBlackInk, options[i], center, kImitationIconRadius);
    }
}

ClickTarget GameView::hitTest(sf::Vector2f point, const Game &game, const InputController &controller,
                               sf::Vector2u windowSize) const
{
    const float windowWidth = static_cast<float>(windowSize.x);

    if (point.y < kTopBarHeight)
    {
        if (controller.requiresExplicitCancel())
        {
            const sf::Vector2f center = actionButtonCenter(0, windowWidth);
            const sf::Vector2f delta = point - center;
            if (std::sqrt(delta.x * delta.x + delta.y * delta.y) <= kActionButtonRadius)
            {
                return ClickTarget{std::nullopt, std::nullopt, std::nullopt, std::nullopt, true, std::nullopt};
            }
            return {}; // top bar, but not over the Cancel button
        }

        const std::vector<ActionKind> options = controller.actionOptions();
        for (std::size_t i = 0; i < options.size(); ++i)
        {
            const sf::Vector2f center = actionButtonCenter(i, windowWidth);
            const sf::Vector2f delta = point - center;
            if (std::sqrt(delta.x * delta.x + delta.y * delta.y) <= kActionButtonRadius)
            {
                return ClickTarget{std::nullopt, std::nullopt, std::nullopt, options[i], false, std::nullopt};
            }
        }
        return {}; // top bar, but not over a button
    }

    // Below the bar, not inside it -- see drawImitationPopup. Checked
    // before hand/board hit-testing since the popup floats over the top
    // of the board area when it's showing.
    const std::vector<PieceType> imitationChoices = controller.imitationOptions();
    for (std::size_t i = 0; i < imitationChoices.size(); ++i)
    {
        const sf::Vector2f center = imitationIconCenter(i, imitationChoices.size(), windowWidth);
        const sf::Vector2f delta = point - center;
        if (std::sqrt(delta.x * delta.x + delta.y * delta.y) <= kImitationIconRadius)
        {
            return ClickTarget{std::nullopt, std::nullopt, std::nullopt, std::nullopt, false, imitationChoices[i]};
        }
    }

    const HandLayout hand = fitHandLayout(static_cast<float>(windowSize.y));
    for (Color color : {Color::White, Color::Black})
    {
        const bool isWhite = color == Color::White;
        const float x = isWhite ? kSideMargin : windowWidth - kSideMargin;
        for (std::size_t i = 0; i < kHandOrder.size(); ++i)
        {
            const sf::Vector2f center{x, hand.topMargin + static_cast<float>(i) * hand.rowSpacing};
            const sf::Vector2f delta = point - center;
            if (std::sqrt(delta.x * delta.x + delta.y * delta.y) <= hand.swatchRadius)
            {
                return ClickTarget{std::nullopt, color, kHandOrder[i], std::nullopt, false, std::nullopt};
            }
        }
    }

    const bool inSidePanel = point.x < kPanelWidth || point.x > windowWidth - kPanelWidth;
    if (inSidePanel)
    {
        return {}; // panel space between/below rows -- not a piece
    }

    const std::vector<Hex> occupied = game.board().occupiedHexes();
    const HexLayout layout = fitLayout(occupied, windowWidth, static_cast<float>(windowSize.y));
    return ClickTarget{layout.pixelToHex(point), std::nullopt, std::nullopt, std::nullopt, false, std::nullopt};
}
