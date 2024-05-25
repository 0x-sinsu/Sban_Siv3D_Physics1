#include <Siv3D.hpp>
#include <fstream>

/// @brief 文字
struct P2Glyph
{
	/// @brief 物理演算の物体
	P2Body body;

	/// @brief 文字のポリゴン
	MultiPolygon polygons;

	/// @brief 文字のポリゴンの凸包
	Polygon convexHull;

	/// @brief 初期位置
	Vec2 initialPos{ 0, 0 };

	/// @brief 落ちる順番
	int32 order = 0;

	void draw(const ColorF& color) const
	{
		if (body)
		{
			const auto [s, c] = FastMath::SinCos(body.getAngle());
			polygons.drawTransformed(s, c, body.getPos(), color);
		}
	}
};

/// @brief 文字にかかるブラックホールの力を計算します。
/// @param blackHolePos ブラックホールの位置
/// @param pos 文字の位置
/// @return ブラックホールの力
Vec2 GetForceToBlackHole(const Vec2& blackHolePos, const Vec2& pos)
{
	const Vec2 v = (blackHolePos - pos);
	const Vec2 dir = v.normalized();
	const double distance = Max((v.length() / 100.0), 0.5); // ブラックホールまでの距離（m）

	if (2.0 < distance) // 2m 以上離れている場合は力が働かない
	{
		return Vec2{ 0, 0 };
	}

	return (dir * 1600 / (distance * distance)); // 距離の二乗に反比例して力が強くなる
}
Vec2 GetForceToBlackHole2(const Vec2& blackHolePos2, const Vec2& pos)
{
	const Vec2 v = (blackHolePos2 - pos);
	const Vec2 dir = v.normalized();
	const double distance = Max((v.length() / 100.0), 0.5); // ブラックホールまでの距離（m）

	if (2.0 < distance) // 2m 以上離れている場合は力が働かない
	{
		return Vec2{ 0, 0 };
	}

	return (dir * 1600 / (distance * distance)); // 距離の二乗に反比例して力が強くなる
}

/// @brief 物理演算用に多角形の凸包を計算します。
/// @param polygons 多角形
/// @return 凸包
static Polygon CalculateConvexHull(const MultiPolygon& polygons)
{
	Array<Vec2> points;

	for (const auto& polygon : polygons)
	{
		for (const auto& point : polygon.outer())
		{
			points << point;
		}
	}

	return Geometry2D::ConvexHull(points).simplified();
}

// 設定を読み込む関数
std::unordered_multimap<std::string, std::string> LoadSettings(const std::string& settingsFilePath) {
	std::unordered_multimap<std::string, std::string> settings;
	std::ifstream file(settingsFilePath);
	std::string line;

	while (std::getline(file, line)) {
		auto delimiterPos = line.find('=');
		if (delimiterPos != std::string::npos) {
			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 1);
			settings.insert({ key, value });
		}
	}
	return settings;
}

s3d::Array<std::string> LoadText(const std::string& filePath) {
	s3d::Array<std::string> lines;
	std::ifstream file(filePath);
	std::string line;

	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}

// std::vector<std::string> を s3d::Array<s3d::String> に変換する関数
s3d::Array<s3d::String> ConvertToS3DArray(const std::vector<std::string>& stdVector) {
	s3d::Array<s3d::String> s3dArray;
	for (const auto& stdString : stdVector) {
		// s3d::String への変換を明示的に行います。
		s3dArray.push_back(s3d::Unicode::FromUTF8(stdString));
	}
	return s3dArray;
}

/// @brief 各文字を生成します。
/// @param bottomCenter 最下層の中心位置
/// @param font フォント
/// @param texts 歌詞
/// @return P2Glyph の配列
static Array<P2Glyph> GenerateGlyphs(const Vec2& bottomCenter, const Font& font, const Array<String>& texts)
{
	Array<P2Glyph> allGlyphs;

	// 下から何行目か
	int32 lineCount = static_cast<int32>(texts.size());

	for (const auto& text : texts)
	{
		const Array<PolygonGlyph> polygonGlyphs = font.renderPolygons(text);
		const Array<bool> isKanji = text.map([](const char32 ch) { return (0x4E00 <= ch); });

		Vec2 basePos{ 0, bottomCenter.y };

		Array<P2Glyph> line;

		for (size_t i = 0; i < polygonGlyphs.size(); ++i)
		{
			const auto& polygonGlyph = polygonGlyphs[i];
			const double scale = (isKanji[i] ? 1.0 : 1.0);
			P2Glyph glyph;
			glyph.polygons = polygonGlyph.polygons.scaled(scale);
			glyph.convexHull = CalculateConvexHull(polygonGlyph.polygons);
			glyph.initialPos = (basePos + polygonGlyph.getBase(scale));
			glyph.order = lineCount;

			basePos.x += (polygonGlyph.xAdvance * scale);
			line << glyph;
		}

		if (line.isEmpty())
		{
			continue;
		}

		// 現在の行の幅
		const double lineLength = basePos.x;
		const double halfLineLength = (lineLength * 0.5);

		// 行を中心揃えする
		for (auto& elem : line)
		{
			elem.initialPos.x -= halfLineLength;
			elem.initialPos.x += bottomCenter.x;
		}

		allGlyphs.insert(allGlyphs.end(), line.begin(), line.end());

		--lineCount;
	}

	return allGlyphs;
}

void Main()
{
  auto result = System::MessageBoxOKCancel(
      U"警告",
      U"このプログラムはフルスクリーンで実行されます。\n("
      U"ウィンドウで実行する場合はコードを書き換えてご自身でビルドしてください)"
      U"\nEscキーを押すと終了します。\n実行してもよろしいですか？");
  if (result == MessageBoxResult::Cancel) {
    return;
  }

	// フルスクリーン
	Window::SetFullscreen(true);
	Scene::SetBackground(ColorF{ 0.0 });

	// std::unordered_multimapの初期化
	std::unordered_multimap<std::string, std::string> settingsMap;

	// 設定ファイルのパス
	const std::string settingsFilePath = "./settings.conf";

	// 設定を読み込む
	auto settings = LoadSettings(settingsFilePath);

	// 設定ファイルの内容を取得
	std::string fontPath;
	auto itFont = settings.find("fontPath");
	if (itFont != settings.end()) {
		fontPath = itFont->second;
	}

	std::string fontSize;
	auto itFontSize = settings.find("fontSize");
	if (itFontSize != settings.end()) {
		fontSize = itFontSize->second;
	}

	s3d::Array<std::string> texts;
	auto itLyrics = settings.find("lyricsPath");
	if (itLyrics != settings.end()) {
		texts = LoadText(itLyrics->second);
	}

	s3d::Array<std::string> left;
	auto itleft = settings.find("left");
	if (itleft != settings.end()) {
		left = LoadText(itleft->second);
	}

	s3d::Array<std::string> right;
	auto itright = settings.find("right");
	if (itright != settings.end()) {
		right = LoadText(itright->second);
	}

	std::string simulationSpeed;
	auto itsimulationSpeed = settings.find("simulationSpeed");
	if (itsimulationSpeed != settings.end()) {
		simulationSpeed = itsimulationSpeed->second;
	}

	std::string frameRate;
    auto itframeRate = settings.find("frameRate");
    if (itframeRate != settings.end()) {
		frameRate = itframeRate->second;
	}

	// s3d::Array<s3d::String> に変換
	s3d::Array<s3d::String> s3dTexts = ConvertToS3DArray(texts);
	s3d::Array<s3d::String> s3dLeft = ConvertToS3DArray(left);
	s3d::Array<s3d::String> s3dRight = ConvertToS3DArray(right);

	// s3d::Stringへ変換
	s3d::String s3dFontPath = s3d::Unicode::FromUTF8(fontPath);

	int intFontSize;
	try {
		intFontSize = std::stoi(fontSize);
	}
	catch (const std::exception) {
		// 変換に失敗した場合の処理(70を使う)
		intFontSize = 70;
	}

	// Fontオブジェクトを初期化
	const Font font(intFontSize, s3dFontPath);

	Array<P2Body> body;

	// シミュレーションスピード
	double Speed = 1.75;
	try {
		if (!simulationSpeed.empty()) {
			Speed = std::stod(simulationSpeed); // 文字列をdoubleに変換
		}
	}
	catch (const std::exception) {
	}

	// フレームレートを設定
	int intFrameRate = 60;
	if (!frameRate.empty()) {
		try {
			intFrameRate = std::stoi(frameRate);
		} catch (const std::exception) {
		}
    }

	int FPS = intFrameRate;

	// 2D 物理演算のシミュレーションステップ（秒）
	constexpr double StepTime = (1.0 / 200.0);

	// 2D 物理演算のシミュレーション蓄積時間（秒）
	double accumulatedTime = 0.0;

	// 基本重力
	constexpr double BaseGravity = 160.0;

	// 2D 物理演算のワールド
	P2World world{ BaseGravity };

	Array<P2Glyph> glyphs = GenerateGlyphs(Vec2{ 0, -1100 }, font, s3dTexts);

	Camera2D camera{ Vec2{ 0, 0 }, 1.0 };

	// ブラックホールの位置
	Vec2 blackHolePos{ -200, -150 };
	Vec2 blackHolePos2{ Scene::Width() + 200, -150 }; // 右端から開始

	// 何番まで登場しているか
	int32 activeOrder = 0;

	// 各行の登場タイミングを決めるためのストップウォッチ
	Stopwatch stopwatch{ StartImmediately::Yes };

	// 文字(ブラックホール)の移動速度（ピクセル/フレーム）
	double dotSpeed = 0; // 初速度は0に設定

	// 文字(ブラックホール)の加速度（ピクセル/フレーム^2）
	const double dotAcceleration = 0.02; // 1フレームごとに加速する量

	// 文字(ブラックホール)の初期位置
	Vec2 dotPos(0, Scene::Height() * 0.5);
	Vec2 dotPos2(Scene::Width(), Scene::Height() * 0.5);

	// 衝突したかどうかのフラグ
	bool isCollided = false;

	// 衝突後の文字の角度と速度
	double collisionAngle = 0.0;
	double collisionSpeed = 0.02;

	const double upwardForce = -1.0; // 上向きの力（負の値で上に向かう）

	while (System::Update())
	{
		// 衝突検出
		if (Abs(dotPos.x - dotPos2.x) < 10 && Abs(dotPos.y - dotPos2.y) < 10 && !isCollided)
		{
			isCollided = true;
			collisionSpeed *= 0.05;
			collisionAngle = Random(Math::Pi * 0.75, Math::Pi * 1.25); // 上向きに衝突角を制限
		}

		// 衝突後の動き
		if (isCollided)
		{
			// より上向きに強くするために、角度を小さめに増やす
			collisionAngle += 0.05;

			// 上向きの力を加えながら、角度に沿って動かす
			dotPos.x += collisionSpeed * std::cos(collisionAngle);
			dotPos.y += collisionSpeed * std::sin(collisionAngle) + upwardForce;

			// 衝突後は、dotPos2もdotPosに追従させる
			dotPos2 = dotPos;
		}

		{
			// 何番まで登場させてよいかを計算する
			const int32 t = static_cast<int32>(((stopwatch.ms() * Speed) - 800) / 500);

			if (activeOrder < t) // 順番が進んだ
			{
				++activeOrder;

				// 同じ順番の文字の物理演算の物体を作成する
				for (auto& glyph : glyphs)
				{
					if (glyph.order == activeOrder)
					{
						// 文字の物理演算の物体を作成する
						glyph.body = world.createPolygon(P2Dynamic, glyph.initialPos, glyph.convexHull);

						// 文字によって重力の倍率を変える
						glyph.body.setGravityScale(Random(1.0, 1.2));

						// 後ろが詰まらないように下向きの初速を与える
						glyph.body.setVelocity(Vec2{ 0, 90 });
					}
				}
			}
		}

		for (accumulatedTime += (Scene::DeltaTime() * Speed); StepTime <= accumulatedTime; accumulatedTime -= StepTime)
		{
			// 2D 物理演算のワールドを StepTime 秒進める
			world.update(StepTime);

			for (auto& glyph : glyphs)
			{
				// 文字にかかるブラックホールからの力を計算する
				const Vec2 force1 = GetForceToBlackHole(blackHolePos, glyph.body.getPos());
				const Vec2 force2 = GetForceToBlackHole(blackHolePos2, glyph.body.getPos());

				// 文字に力を加える
				glyph.body.applyForce(force1 + force2); // 両方の力を合成して適用
			}
		}

		// 文字(ブラックホール)の速度を加速度分だけ増加させる
		dotSpeed += dotAcceleration;

		// 文字(ブラックホール)の位置を更新する
		dotPos.x += dotSpeed;
		dotPos2.x -= dotSpeed;

		camera.update();
		{
			const auto t = camera.createTransformer();

			// 各 P2Glyph の描画
			for (const auto& glyph : glyphs)
			{
				glyph.draw(ColorF{ 0.93 });
			}
		}

		// ブラックホールの位置を更新
		blackHolePos = camera.getCenter() + Vec2(dotPos.x - Scene::Width() * 0.5, 0);
		blackHolePos2 = camera.getCenter() + Vec2(dotPos2.x - Scene::Width() * 0.5, 0);

		// dotPosで描画する文字
		font(s3dLeft).drawAt(dotPos, Palette::White);

		// dotPos2で描画する文字
		font(s3dRight).drawAt(dotPos2, Palette::White);
		
		// ブラックホールを描画する（カメラ座標を考慮せずスクリーン座標で描画）
		Circle(dotPos, 10).draw(ColorF(0.0, 0.0, 0.0, 0.0));
		Circle(dotPos2, 10).draw(ColorF(0.0, 0.0, 0.0, 0.0));
	}
}
