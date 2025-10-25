#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <kson/util/graph_utils.hpp>

TEST_CASE("BakeStopIntoScrollSpeed with no stop", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 1.0 };
	scrollSpeed[960] = kson::GraphValue{ 2.0 };

	kson::ByPulse<kson::RelPulse> stop;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.size() == 2);
	REQUIRE(result[0].v.v == Approx(1.0));
	REQUIRE(result[960].v.v == Approx(2.0));
}

TEST_CASE("BakeStopIntoScrollSpeed with simple stop", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 1.0 };

	kson::ByPulse<kson::RelPulse> stop;
	stop[0] = 192;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.count(0) == 1);
	REQUIRE(result[0].v.v == Approx(1.0));
	REQUIRE(result[0].v.vf == Approx(0.0));

	REQUIRE(result.count(192) == 1);
	REQUIRE(result[192].v.v == Approx(0.0));
	REQUIRE(result[192].v.vf == Approx(1.0));
}

TEST_CASE("BakeStopIntoScrollSpeed with scroll_speed changes inside stop", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 1.0 };
	scrollSpeed[48] = kson::GraphValue{ -1.0 };
	scrollSpeed[96] = kson::GraphValue{ -1.0 };
	scrollSpeed[144] = kson::GraphValue{ 1.0 };

	kson::ByPulse<kson::RelPulse> stop;
	stop[0] = 192;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.count(48) == 0);
	REQUIRE(result.count(96) == 0);
	REQUIRE(result.count(144) == 0);

	REQUIRE(result.count(0) == 1);
	REQUIRE(result.count(192) == 1);

	REQUIRE(result[0].v.v == Approx(1.0));
	REQUIRE(result[0].v.vf == Approx(0.0));

	REQUIRE(result[192].v.v == Approx(0.0));
	REQUIRE(result[192].v.vf == Approx(1.0));
}

TEST_CASE("BakeStopIntoScrollSpeed with multiple stops", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 2.0 };

	kson::ByPulse<kson::RelPulse> stop;
	stop[200] = 100;
	stop[500] = 100;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.count(200) == 1);
	REQUIRE(result[200].v.v == Approx(2.0));
	REQUIRE(result[200].v.vf == Approx(0.0));

	REQUIRE(result.count(300) == 1);
	REQUIRE(result[300].v.v == Approx(0.0));
	REQUIRE(result[300].v.vf == Approx(2.0));

	REQUIRE(result.count(500) == 1);
	REQUIRE(result[500].v.v == Approx(2.0));
	REQUIRE(result[500].v.vf == Approx(0.0));

	REQUIRE(result.count(600) == 1);
	REQUIRE(result[600].v.v == Approx(0.0));
	REQUIRE(result[600].v.vf == Approx(2.0));
}

TEST_CASE("BakeStopIntoScrollSpeed with overlapping stops", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 1.0 };

	kson::ByPulse<kson::RelPulse> stop;
	stop[400] = 200;
	stop[500] = 300;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.count(400) == 1);
	REQUIRE(result[400].v.v == Approx(1.0));
	REQUIRE(result[400].v.vf == Approx(0.0));

	REQUIRE(result.count(500) == 0);
	REQUIRE(result.count(600) == 0);

	REQUIRE(result.count(800) == 1);
	REQUIRE(result[800].v.v == Approx(0.0));
	REQUIRE(result[800].v.vf == Approx(1.0));
}

TEST_CASE("BakeStopIntoScrollSpeed with stop during scroll_speed transition", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 1.0 };
	scrollSpeed[1000] = kson::GraphValue{ 3.0 };

	kson::ByPulse<kson::RelPulse> stop;
	stop[400] = 200;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.count(400) == 1);
	REQUIRE(result[400].v.v == Approx(1.8));
	REQUIRE(result[400].v.vf == Approx(0.0));

	REQUIRE(result.count(600) == 1);
	REQUIRE(result[600].v.v == Approx(0.0));
	REQUIRE(result[600].v.vf == Approx(2.2));

	REQUIRE(result.count(1000) == 1);
	REQUIRE(result[1000].v.v == Approx(3.0));
}

TEST_CASE("BakeStopIntoScrollSpeed with vf mismatch at start point", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 1.0 };
	scrollSpeed[100] = kson::GraphValue{ 1.0, -1.0 };
	scrollSpeed[200] = kson::GraphValue{ 1.0 };

	kson::ByPulse<kson::RelPulse> stop;
	stop[100] = 150;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.count(100) == 1);
	REQUIRE(result[100].v.v == Approx(-1.0));
	REQUIRE(result[100].v.vf == Approx(0.0));

	REQUIRE(result.count(200) == 0);

	REQUIRE(result.count(250) == 1);
	REQUIRE(result[250].v.v == Approx(0.0));
	REQUIRE(result[250].v.vf == Approx(1.0));
}

TEST_CASE("BakeStopIntoScrollSpeed with scroll_speed change after stop", "[stop][scroll_speed]")
{
	kson::Graph scrollSpeed;
	scrollSpeed[0] = kson::GraphValue{ 1.0 };
	scrollSpeed[1920] = kson::GraphValue{ -10.0 };
	scrollSpeed[3840] = kson::GraphValue{ 1.0 };

	kson::ByPulse<kson::RelPulse> stop;
	stop[960] = 480;

	kson::Graph result = kson::BakeStopIntoScrollSpeed(scrollSpeed, stop);

	REQUIRE(result.count(960) == 1);
	REQUIRE(result[960].v.v == Approx(-4.5));
	REQUIRE(result[960].v.vf == Approx(0.0));

	REQUIRE(result.count(1440) == 1);
	REQUIRE(result[1440].v.v == Approx(0.0));
	REQUIRE(result[1440].v.vf == Approx(-7.25));

	REQUIRE(result.count(1920) == 1);
	REQUIRE(result[1920].v.v == Approx(-10.0));

	REQUIRE(result.count(3840) == 1);
	REQUIRE(result[3840].v.v == Approx(1.0));
}
