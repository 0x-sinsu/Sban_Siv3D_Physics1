# include <Siv3D.hpp> // Siv3D v0.6.13

int main() {
	// 終了フラグ
	std::atomic<bool> done(false);

	// 5分後に終了フラグをセットする非同期タスク
	auto future = std::async(std::launch::async, [&done]() {
		std::this_thread::sleep_for(std::chrono::minutes(5));
		done = true;
	});

	return 0;
}

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
	const char32_t* fontPath = U"C:/VisualStudioProject/P2Glyph_1/SourceHanSansJP-Heavy.otf";
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
	// ここ(2行)を編集します。1行目は文字の角度です。
	// 2行目は文字の速度です。
	double collisionAngle = 0.0;
	double collisionSpeed = 0.02;

	// ここを編集します。負の値が大きいほど左右から出てくる文字が衝突後、より上に向かう...はずです。
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

		// 画面の幅と高さを取得
		const double screenWidth = Scene::Width();
		const double screenHeight = Scene::Height();

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

		// 文字を描画（カメラ座標を考慮せずスクリーン座標で描画）
		// ここを編集します。U"文字"です。動かしたい文字(横から出てくる文字)を入力します。
		// 別の場所(もっと下)から出したいなどの場合は説明が長くなってしまうため各自で調べてください。
		font(U".").drawAt(dotPos, Palette::White);
		font(U".").drawAt(dotPos2, Palette::White);

		// ブラックホールを描画する（カメラ座標を考慮せずスクリーン座標で描画）
		Circle(dotPos, 10).draw(ColorF(0.0, 0.0, 0.0, 0.0));
		Circle(dotPos2, 10).draw(ColorF(0.0, 0.0, 0.0, 0.0));


		// 2D カメラの操作を描画する
		camera.draw(Palette::Orange);
	}
}
