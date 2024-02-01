# include <Siv3D.hpp> // Siv3D v0.6.13

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

	// 2D 物理演算のワールド
	P2World world;

	void draw(const ColorF& color) const
	{
		if (body)
		{
			const auto [s, c] = FastMath::SinCos(body.getAngle());
			polygons.drawTransformed(s, c, body.getPos(), color);
		}
	}
};

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
			// ↓ここを編集します。1.25は漢字、1.0はひらがなです。漢字側の比率を調節してください。
			const double scale = (isKanji[i] ? 1.25 : 1.0);

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
	// ↓ここ(2行)を編集します。fontSizeは基準となるフォントサイズです。先程の比率はこれに基づきます。
	// fontPathは使用したいフォントファイルのパスです。区切りには「/」or「\\」を使用します。
	// ヒラギノに類似するフォントである源ノ角ゴシックのダウンロード先はこちら:https://github.com/adobe-fonts/source-han-sans/tree/release/OTF/Japanese
	const int fontSize = 40;
	const char32_t* fontPath = U"fontfilepath(絶対パス,拡張子含む)";
	const Font font{ fontSize, fontPath };

	Window::SetFullscreen(true);
	Scene::SetBackground(ColorF{ 0.0 });

	// シミュレーションスピード
	// ここを編集します。単純にスピードです。
	constexpr double Speed = 1.5;

	// 2D 物理演算のシミュレーションステップ（秒）
	constexpr double StepTime = (1.0 / 200.0);

	// 2D 物理演算のシミュレーション蓄積時間（秒）
	double accumulatedTime = 0.0;

	// 基本重力
	constexpr double BaseGravity = 160.0;

	// 2D 物理演算のワールド
	P2World world{ BaseGravity };

	// 画面の幅と高さ
	const double screenWidth = Scene::Width();
	const double screenHeight = Scene::Height();

	//	↓ここを編集します。固定するテキストを記述します。
	const Array<String> fixedtext =
	{
		// template
		U"全てあなたの所為です。",
	};
	// テキストの幅と高さを計算
	const double textWidth = font(fixedtext).region().w;
	const double textHeight = font(fixedtext).region().h;

	// テキストを画面の中央に配置するための座標を計算
	const Vec2 textPos((screenWidth - textWidth) / 2, (screenHeight - textHeight) / 2);

	// 出力されたP2Glyphの配列を処理して物理ワールドに追加する例
	Array<P2Glyph> glyph2 = GenerateGlyphs(Vec2{ 0, 0 }, font, fixedtext);

	for (auto& glyph : glyph2)
	{
		// 物理ボディを物理ワールドに追加
		glyph.body = world.createPolygon(P2Static, glyph.initialPos, glyph.convexHull);
	}

	// 歌詞
	// ここを編集します。文法は次の通りです:
	// U"歌詞",
	// 「歌詞」の部分に上から落とす文字を入れます。
	// 改行も可能です。
	const Array<String> texts =
	{
		// template
		U"心地よい音　頭蓋の中、",
		U"ひとりでに骨が折れ、",
		U"たわむれに描いた傘の中、",
		U"全てあなたの所為です。",
	};

	Array<P2Glyph> glyphs = GenerateGlyphs(Vec2{ 0, -1100 }, font, texts);

	Camera2D camera{ Vec2{ 0, 0 }, 1.0 };

	// 何番まで登場しているか
	int32 activeOrder = 0;

	// 各行の登場タイミングを決めるためのストップウォッチ
	Stopwatch stopwatch{ StartImmediately::Yes };

	while (System::Update())
	{
		// Escキーで終了
		if (KeyEscape.pressed())
		{
			break;
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

		camera.update();
		{
			const auto t = camera.createTransformer();

			// 各 P2Glyph の描画
			for (const auto& glyph : glyphs)
			{
				glyph.draw(ColorF{ 0.93 });
			}
			for (const auto& glyph : glyph2)
			{
				glyph.draw(ColorF{ 0.93 });
			}
		}

		for (accumulatedTime += (Scene::DeltaTime() * Speed); StepTime <= accumulatedTime; accumulatedTime -= StepTime)
		{
			// 2D 物理演算のワールドを StepTime 秒進める
			world.update(StepTime);
		}

		// 2D カメラの操作を描画する
		camera.draw(Palette::Orange);
	}
}
